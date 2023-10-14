////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/ex/address.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/macros.h>
#include <wex/factory/stc-undo.h>
#include <wex/stc/auto-complete.h>
#include <wex/stc/auto-indent.h>
#include <wex/stc/entry-dialog.h>
#include <wex/stc/stc.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/syntax/printout.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/item-vector.h>
#include <wx/app.h>
#include <wx/settings.h>

#include <algorithm>

wex::stc::stc(const wex::path& p, const data::stc& data)
  : syntax::stc(data.window())
  , m_data(data)
  , m_auto_complete(new wex::auto_complete(this))
  , m_vi(new vi(
      this,
      data.flags().test(data::stc::WIN_EX) ? ex::mode_t::EX :
                                             ex::mode_t::VISUAL))
  , m_file(this, wex::path(data.window().name()))
  , m_hexmode(hexmode(this))
  , m_frame(dynamic_cast<frame*>(wxTheApp->GetTopWindow()))
  , m_function_repeat(
      "stc",
      this,
      [this](wxTimerEvent&)
      {
        if (
          is_visual() && m_file.check_sync() &&
          // the readonly flags bit of course can differ from file actual
          // readonly mode, therefore add this check
          !m_data.flags().test(data::stc::WIN_READ_ONLY) &&
          path().stat().is_readonly() != GetReadOnly())
        {
          file_readonly_attribute_changed();
        }
      })
{
  m_data.set_stc(this);

  assert(m_frame != nullptr);

  if (m_config_items == nullptr)
  {
    on_init();
  }

  get_lexer().set(lexer(this));

  if (p != wex::ex::get_macros().path())
  {
    sync();
  }

#ifdef __WXMSW__
  SetEOLMode(wxSTC_EOL_CRLF);
#elif __WXGTK__
  SetEOLMode(wxSTC_EOL_LF);
#else
  SetEOLMode(wxSTC_EOL_CR);
#endif

  SetAdditionalCaretsBlink(true);
  SetAdditionalCaretsVisible(true);
  SetAdditionalSelectionTyping(true);
  SetBackSpaceUnIndents(true);
  SetMouseDwellTime(1000);

  SetMarginType(m_margin_line_number, wxSTC_MARGIN_NUMBER);
  SetMarginType(m_margin_divider_number, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_margin_folding_number, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_margin_text_number, wxSTC_MARGIN_TEXT);

  SetMarginMask(m_margin_folding_number, wxSTC_MASK_FOLDERS);

  SetMarginSensitive(m_margin_divider_number, true);
  SetMarginSensitive(m_margin_folding_number, true);
  SetMarginSensitive(m_margin_text_number, true);

  SetMultiPaste(wxSTC_MULTIPASTE_EACH);
  SetMultipleSelection(true);

  if (m_zoom == -1)
  {
    m_zoom = GetZoom();
  }
  else
  {
    SetZoom(m_zoom);
  }

  // we have our own popup
  UsePopUp(wxSTC_POPUP_NEVER);

  bind_all();

  if (p.stat().is_ok())
  {
    open(p, data);
  }
  else
  {
    get_lexer().set(path_lexer(p).lexer(), true);

    m_file.file_new(p);
    m_data.inject();
  }
}

wex::stc::stc(const std::string& text, const data::stc& data)
  : stc(wex::path(), data)
{
  if (!text.empty())
  {
    is_hexmode() ? m_hexmode.append_text(text) : set_text(text);
    guess_type_and_modeline();
  }
}

wex::stc::~stc()
{
  delete m_auto_complete;
  delete m_vi;
}

void wex::stc::add_text(const std::string& text)
{
  if (m_vi->visual() == ex::mode_t::EX)
  {
    m_file.ex_stream()->insert_text(
      address(m_vi, m_file.ex_stream()->get_current_line()),
      text,
      ex_stream::loc_t::AFTER);
  }
  else if (!GetOvertype())
  {
    if (text == "\t" && !GetUseTabs())
    {
      Tab();
    }
    else
    {
      Allocate(GetTextLength() + text.size());
      AddTextRaw(text.data(), text.size());
    }
  }
  else
  {
    SetTargetRange(GetCurrentPos(), GetCurrentPos() + text.size());
    ReplaceTarget(text);
  }
}

void wex::stc::add_text_block(const std::string& text)
{
  stc_undo undo(this);

  std::for_each(
    text.begin(),
    text.end(),
    [this](const auto& it)
    {
      if (it != '\n' && it != '\r')
      {
        AddText(it);
      }
      else
      {
        LineDown();
      }
    });
}

void wex::stc::append_text(const std::string& text)
{
  Allocate(GetTextLength() + text.size());
  AppendTextRaw(text.data(), text.size());
}

bool wex::stc::auto_indentation(int c)
{
  return auto_indent(this).on_char(c);
}

bool wex::stc::CanCut() const
{
  return syntax::stc::CanCut() && !GetReadOnly() && !is_hexmode();
}

bool wex::stc::CanPaste() const
{
  return syntax::stc::CanPaste() && !GetReadOnly() && !is_hexmode();
}

void wex::stc::Clear()
{
  m_vi->is_active() && GetSelectedText().empty() ?
    (void)m_vi->command(std::string(1, WXK_DELETE)) :
    syntax::stc::Clear();
}

void wex::stc::Copy()
{
  if (CanCopy())
  {
    syntax::stc::Copy();

    m_selection_mode_copy = GetSelectionMode();
  }
}

void wex::stc::Cut()
{
  if (CanCut())
  {
    if (get_vi().is_active())
    {
      ex::set_registers_delete(get_selected_text());
      ex::set_register_yank(get_selected_text());
    }

    syntax::stc::Cut();
  }
}

bool wex::stc::file_readonly_attribute_changed()
{
  SetReadOnly(path().is_readonly()); // does not return anything
  log::status(_("Readonly attribute changed"));

  return true;
}

int wex::stc::get_current_line() const
{
  if (m_vi->visual() == ex::mode_t::EX)
  {
    return m_file.ex_stream()->get_current_line();
  }
  else
  {
    return syntax::stc::get_current_line();
  }
}

const std::string wex::stc::get_find_string() const
{
  if (const auto selection =
        const_cast<stc*>(this)->GetSelectedText().ToStdString();
      !selection.empty() && get_number_of_lines(selection) == 1)
  {
    bool alnum = true;

    // If regexp is true, then only use selected text if text does not
    // contain special regexp characters.
    if (GetSearchFlags() & wxSTC_FIND_REGEXP)
    {
      for (size_t i = 0; i < selection.size() && alnum; i++)
      {
        if (
          !isalnum(selection[i]) && selection[i] != ' ' &&
          selection[i] != '.' && selection[i] != '-' && selection[i] != '_')
        {
          alnum = false;
        }
      }
    }

    if (alnum)
    {
      find_replace_data::get()->set_find_string(selection);
    }
  }

  return find_replace_data::get()->get_find_string();
}

bool wex::stc::get_hexmode_erase(int begin, int end)
{
  return m_hexmode.erase(begin, end);
}

bool wex::stc::get_hexmode_insert(const std::string& command, int pos)
{
  return m_hexmode.insert(command, pos);
}

std::string wex::stc::get_hexmode_lines(const std::string& text) const
{
  return m_hexmode.lines(text);
}

bool wex::stc::get_hexmode_replace(char c)
{
  return m_hexmode.replace(c);
}

bool wex::stc::get_hexmode_replace_target(
  const std::string& replacement,
  bool               set_text)
{
  return m_hexmode.replace_target(replacement, set_text);
}

bool wex::stc::get_hexmode_sync()
{
  return m_hexmode.sync();
}

int wex::stc::get_line_count() const
{
  if (m_vi->visual() == ex::mode_t::EX)
  {
    return m_file.ex_stream()->get_line_count();
  }
  else
  {
    return syntax::stc::get_line_count();
  }
}

int wex::stc::get_line_count_request()
{
  if (m_vi->visual() == ex::mode_t::EX)
  {
    return m_file.ex_stream()->get_line_count_request();
  }
  else
  {
    return syntax::stc::get_line_count_request();
  }
}

const wex::vi& wex::stc::get_vi() const
{
  return *m_vi;
}

wex::vi& wex::stc::get_vi()
{
  return *m_vi;
}

void wex::stc::goto_line(int line)
{
  if (m_vi->visual() == ex::mode_t::EX)
  {
    m_file.ex_stream()->goto_line(line);
  }
  else
  {
    syntax::stc::goto_line(line);
  }
}

void wex::stc::guess_type_and_modeline()
{
  // Get a small sample from this document to detect the file mode.
  const auto length(
    (!is_hexmode() ? GetTextLength() : m_hexmode.buffer().size()));
  const auto sample_size((length > 255 ? 255 : length));
  const auto head(
    (!is_hexmode() ? GetTextRange(0, sample_size).ToStdString() :
                     m_hexmode.buffer().substr(0, sample_size)));
  const auto tail(
    (!is_hexmode() ?
       GetTextRange(length - sample_size, length).ToStdString() :
       m_hexmode.buffer().substr(length - sample_size, sample_size)));

  // If we have a modeline comment.
  if (regex v("\\s+vim?:\\s*(set [a-z0-9:= ]+)");
      get_vi().is_active() && (v.search(head) > 0 || v.search(tail) > 0))
  {
    if (!get_vi().command(
          // the vim modeline command sometimes ends with a : (see test)
          ":" + rfind_before(v[0], ":") + "*")) // add * to indicate modeline
    {
      log::status("Could not apply vi settings");
    }
  }

  if (head.contains("\r\n"))
    SetEOLMode(wxSTC_EOL_CRLF);
  else if (head.contains("\n"))
    SetEOLMode(wxSTC_EOL_LF);
  else if (head.contains("\r"))
    SetEOLMode(wxSTC_EOL_CR);
  else
    return; // do nothing

  m_frame->update_statusbar(this, "PaneFileType");
}

bool wex::stc::inject(const data::control& data)
{
  return m_data.control(data).inject();
}

void wex::stc::insert_text(int pos, const std::string& text)
{
  if (m_vi->visual() == ex::mode_t::EX)
  {
    m_file.ex_stream()->insert_text(address(m_vi, LineFromPosition(pos)), text);
  }
  else
  {
    InsertText(pos, text);
  }
}

bool wex::stc::IsModified() const
{
  return is_visual() ? GetModify() : m_file.ex_stream()->is_modified();
}

bool wex::stc::is_visual() const
{
  return m_vi->visual() != ex::mode_t::EX;
}

bool wex::stc::marker_delete_all_change()
{
  if (!lexers::get()->marker_is_loaded(m_marker_change))
  {
    return false;
  }

  MarkerDeleteAll(m_marker_change.number());

  return true;
}

void wex::stc::mark_modified(const wxStyledTextEvent& event)
{
  if (!lexers::get()->marker_is_loaded(m_marker_change))
  {
    return;
  }

  use_modification_markers(false);

  if (const auto line = LineFromPosition(event.GetPosition());
      event.GetModificationType() & wxSTC_PERFORMED_UNDO)
  {
    if (event.GetLinesAdded() == 0)
    {
      MarkerDelete(line, m_marker_change.number());
    }
    else
    {
      for (int i = 0; i < abs(event.GetLinesAdded()); i++)
      {
        MarkerDelete(line + 1, m_marker_change.number());
      }
    }

    if (!IsModified())
    {
      marker_delete_all_change();
    }
  }
  else if (
    (event.GetModificationType() & wxSTC_MOD_INSERTTEXT) ||
    (event.GetModificationType() & wxSTC_MOD_DELETETEXT))
  {
    if (event.GetLinesAdded() <= 0)
    {
      MarkerAdd(line, m_marker_change.number());
    }
    else
    {
      for (int i = 0; i < event.GetLinesAdded(); i++)
      {
        MarkerAdd(line + i, m_marker_change.number());
      }
    }
  }

  use_modification_markers(true);
}

void wex::stc::on_styled_text(wxStyledTextEvent& event)
{
  if (is_visual())
  {
    mark_modified(event);
  }

  event.Skip();
}

bool wex::stc::open(const wex::path& p, const data::stc& data)
{
  m_data = data::stc(data).window(data::window().name(p.string()));
  m_data.set_stc(this);

  if (path() != p)
  {
    if (!m_file.file_load(p))
    {
      return false;
    }
  }
  else
  {
    m_data.inject();
  }

  if (data.recent())
  {
    m_frame->set_recent_file(p);
  }

  return true;
}

void wex::stc::Paste()
{
  if (CanPaste())
  {
    if (m_selection_mode_copy == wxSTC_SEL_RECTANGLE)
    {
      add_text_block(clipboard_get());
    }
    else
    {
      syntax::stc::Paste();
    }
  }
}

void wex::stc::print(bool prompt)
{
  auto* data = printing::get()->get_html_printer()->GetPrintData();
  printing::get()->get_printer()->GetPrintDialogData().SetPrintData(*data);
  printing::get()->get_printer()->Print(this, new printout(this), prompt);
}

void wex::stc::print_preview(wxPreviewFrameModalityKind kind)
{
  auto* preview = new wxPrintPreview(new printout(this), new printout(this));

  if (!preview->Ok())
  {
    delete preview;
    stc_entry_dialog("There was a problem previewing.\n"
                     "Perhaps your current printer is not set correctly?")
      .ShowModal();
    return;
  }

  auto* frame = new wxPreviewFrame(
    preview,
    this,
    print_caption(wex::path(GetName().ToStdString())));

  frame->InitializeWithModality(kind);
  frame->Show();
}

void wex::stc::properties_message(path::log_t flags)
{
  log::status() << wex::path(path().string(), flags);

  if (!flags[path::LOG_SYNC])
  {
    m_frame->update_statusbar(this, "PaneFileType");
    m_frame->update_statusbar(this, "PaneLexer");
    m_frame->update_statusbar(this, "PaneMode");
  }

  m_frame->update_statusbar(this, "PaneInfo");

  if (!flags[path::LOG_SYNC])
  {
    const auto name(
      GetName().empty() ? path().string() : GetName().ToStdString());

    const auto readonly(
      GetReadOnly() ? std::string(" [" + _("Readonly") + "]") : std::string());

    std::string title = name + readonly;

    if (m_vi->visual() == ex::mode_t::EX)
    {
      title += " [ex]";
    }

    m_frame->SetTitle(
      !title.empty() ? title : wxTheApp->GetAppName().ToStdString());
  }
}

void wex::stc::SelectNone()
{
  // The base styledtextctrl version uses scintilla, sets caret at 0.
  wxTextEntryBase::SelectNone();
}

void wex::stc::set_search_flags(int flags)
{
  if (flags == -1)
  {
    flags = 0;

    auto* frd = find_replace_data::get();

    if (frd->is_regex())
      flags |= wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX;

    if (frd->match_word() && !frd->is_regex())
      flags |= wxSTC_FIND_WHOLEWORD;

    if (frd->match_case())
      flags |= wxSTC_FIND_MATCHCASE;
  }

  SetSearchFlags(flags);
}

void wex::stc::set_text(const std::string& value)
{
  clear();

  add_text(value);

  DocumentStart();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

void wex::stc::show_line_numbers(bool show)
{
  if (is_visual())
  {
    const item_vector iv(m_config_items);

    SetMarginWidth(
      m_margin_line_number,
      show ? iv.find<int>(_("stc.margin.Line number")) : 0);
  }
}

void wex::stc::Undo()
{
  syntax::stc::Undo();
  m_hexmode.undo();
}

void wex::stc::use_modification_markers(bool use)
{
  use ? Bind(wxEVT_STC_MODIFIED, &stc::on_styled_text, this) :
        (void)Unbind(wxEVT_STC_MODIFIED, &stc::on_styled_text, this);
}

bool wex::stc::vi_command(const line_data& data)
{
  m_vi->set_line_data(data);

  const bool r(m_vi->command(data.command()));

  m_vi->set_line_data(line_data());

  return r;
}

bool wex::stc::vi_command_finish(bool user_input)
{
  return m_vi->command_finish(user_input);
}

bool wex::stc::vi_is_visual() const
{
  return m_vi->mode().is_visual();
}

const std::string wex::stc::vi_mode() const
{
  return m_vi->mode().str();
}

void wex::stc::vi_record(const std::string& command)
{
  ex::get_macros().record(command);
}

std::string wex::stc::vi_register(char c) const
{
  return ex::get_macros().get_register(c);
}

void wex::stc::visual(bool on)
{
  SetReadOnly(!on);

  m_data.flags(
    data::stc::window_t().set(data::stc::WIN_EX),
    on ? data::control::NOT : data::control::SET);

  if (on)
  {
    if (m_vi->visual() != ex::mode_t::VISUAL)
    {
      std::stringstream info;

      if (!path().string().empty())
      {
        info << path().string();
      }

      log::info("enter visual mode") << on << info;
    }

    m_vi->use(ex::mode_t::VISUAL); // needed in do_file_load
    m_file.close();
    m_file.use_stream(false);
    m_file.file_load(path());
  }
  else
  {
    m_vi->use(ex::mode_t::EX);
  }

  config_get();

  m_frame->show_ex_bar(!on ? frame::SHOW_BAR : frame::HIDE_BAR_FOCUS_STC, this);
}
