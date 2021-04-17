////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/address.h>
#include <wex/blame.h>
#include <wex/config.h>
#include <wex/ex-stream.h>
#include <wex/frame.h>
#include <wex/frd.h>
#include <wex/indicator.h>
#include <wex/item-vector.h>
#include <wex/lexers.h>
#include <wex/link.h>
#include <wex/macros.h>
#include <wex/path.h>
#include <wex/printing.h>
#include <wex/regex.h>
#include <wex/stc-entry-dialog.h>
#include <wex/stc.h>
#include <wex/vcs-entry.h>
#include <wx/app.h>
#include <wx/settings.h>

wex::stc::stc(const path& p, const data::stc& data)
  : m_data(this, data)
  , m_auto_complete(this)
  , m_vi(
      new vi(this, data.flags().test(data::stc::WIN_EX) ? ex::EX : ex::VISUAL))
  , m_file(this, data.window().name())
  , m_hexmode(hexmode(this))
  , m_frame(dynamic_cast<frame*>(wxTheApp->GetTopWindow()))
{
  assert(m_frame != nullptr);

  Create(
    data.window().parent(),
    data.window().id(),
    data.window().pos(),
    data.window().size(),
    data.window().style(),
    data.window().name());

  if (m_config_items == nullptr)
  {
    on_init();
  }

  get_lexer().set(lexer(this));

  if (
    config("AllowSync").get(true) && p != wex::ex::get_macros().get_filename())
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
  : stc(path(), data)
{
  if (!text.empty())
  {
    is_hexmode() ? m_hexmode.append_text(text) : set_text(text);
    guess_type_and_modeline();
  }

  m_data.inject();
}

wex::stc::~stc()
{
  delete m_vi;
}

void wex::stc::add_text(const std::string& text)
{
  if (m_vi->visual() == ex::EX)
  {
    m_file.ex_stream()->insert_text(
      address(m_vi, m_file.ex_stream()->get_current_line()),
      text,
      ex_stream::INSERT_AFTER);
  }
  else if (!GetOvertype())
  {
    Allocate(GetTextLength() + text.size());
    AddTextRaw(text.data(), text.size());
  }
  else
  {
    SetTargetRange(GetCurrentPos(), GetCurrentPos() + text.size());
    ReplaceTarget(text);
  }
}

void wex::stc::append_text(const std::string& text)
{
  Allocate(GetTextLength() + text.size());
  AppendTextRaw(text.data(), text.size());
}

void wex::stc::auto_complete_clear()
{
  m_auto_complete.clear();
}

void wex::stc::auto_complete_sync()
{
  m_auto_complete.sync();
}

bool wex::stc::CanCut() const
{
  return factory::stc::CanCut() && !GetReadOnly() && !is_hexmode();
}

bool wex::stc::CanPaste() const
{
  return factory::stc::CanPaste() && !GetReadOnly() && !is_hexmode();
}

void wex::stc::Clear()
{
  m_vi->is_active() && GetSelectedText().empty() ?
    (void)m_vi->command(std::string(1, WXK_DELETE)) :
    factory::stc::Clear();
}

void wex::stc::clear(bool set_savepoint)
{
  SetReadOnly(false);

  ClearAll();

  if (set_savepoint)
  {
    EmptyUndoBuffer();
    SetSavePoint();
  }
}

void wex::stc::Copy()
{
  if (CanCopy())
  {
    factory::stc::Copy();
  }
}

void wex::stc::Cut()
{
  if (CanCut())
  {
    if (get_vi().is_active())
    {
      get_vi().set_registers_delete(get_selected_text());
      get_vi().set_register_yank(get_selected_text());
    }

    factory::stc::Cut();
  }
}

bool wex::stc::file_readonly_attribute_changed()
{
  SetReadOnly(get_filename().is_readonly()); // does not return anything
  log::status(_("Readonly attribute changed"));

  return true;
}

void wex::stc::fold(bool all)
{
  if (const item_vector & iv(m_config_items);
      all || get_line_count() > iv.find<int>(_("stc.Auto fold")))
  {
    fold_all();
  }
}

void wex::stc::fold_all()
{
  if (GetProperty("fold") != "1")
    return;

  const auto current_line = get_current_line();
  const bool json         = (get_lexer().scintilla_lexer() == "json");
  const bool xml          = (get_lexer().language() == "xml");

  int line = (json ? 1 : 0);

  while (line < get_line_count())
  {
    if (const auto level = GetFoldLevel(line);
        xml && (level == wxSTC_FOLDLEVELBASE + wxSTC_FOLDLEVELHEADERFLAG))
    {
      line++;
    }
    else if (const auto last_child_line = GetLastChild(line, level);
             last_child_line > line + 1)
    {
      if (GetFoldExpanded(line))
      {
        ToggleFold(line);
      }

      line = last_child_line + 1;
    }
    else
    {
      line++;
    }
  }

  goto_line(current_line);
}

int wex::stc::get_current_line() const
{
  if (m_vi->visual() == ex::EX)
  {
    return m_file.ex_stream()->get_current_line();
  }
  else
  {
    return factory::stc::get_current_line();
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
  if (m_vi->visual() == ex::EX)
  {
    return m_file.ex_stream()->get_line_count();
  }
  else
  {
    return factory::stc::get_line_count();
  }
}

int wex::stc::get_line_count_request()
{
  if (m_vi->visual() == ex::EX)
  {
    return m_file.ex_stream()->get_line_count_request();
  }
  else
  {
    return factory::stc::get_line_count_request();
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

const std::string wex::stc::get_word_at_pos(int pos) const
{
  const auto word_start = const_cast<stc*>(this)->WordStartPosition(pos, true);
  const auto word_end   = const_cast<stc*>(this)->WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const std::string word = const_cast<stc*>(this)
                               ->GetTextRange(word_start, word_start + 1)
                               .ToStdString();

    return !isspace(word[0]) ? word : std::string();
  }
  else
  {
    return const_cast<stc*>(this)
      ->GetTextRange(word_start, word_end)
      .ToStdString();
  }
}

void wex::stc::goto_line(int line)
{
  if (m_vi->visual() == ex::EX)
  {
    m_file.ex_stream()->goto_line(line);
  }
  else
  {
    factory::stc::goto_line(line);
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
    if (!get_vi().command(":" + v[0] + "*")) // add * to indicate modeline
    {
      log::status("Could not apply vi settings");
    }
  }

  if (head.find("\r\n") != std::string::npos)
    SetEOLMode(wxSTC_EOL_CRLF);
  else if (head.find("\n") != std::string::npos)
    SetEOLMode(wxSTC_EOL_LF);
  else if (head.find("\r") != std::string::npos)
    SetEOLMode(wxSTC_EOL_CR);
  else
    return; // do nothing

  m_frame->update_statusbar(this, "PaneFileType");
}

void wex::stc::insert_text(int pos, const std::string& text)
{
  if (m_vi->visual() == ex::EX)
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
  return m_vi->visual() == ex::VISUAL;
}

bool wex::stc::link_open()
{
  return link_open(link_t().set(LINK_OPEN).set(LINK_OPEN_MIME));
}

bool wex::stc::link_open(link_t mode, std::string* filename)
{
  const auto sel = GetSelectedText().ToStdString();

  if (sel.size() > 200 || (!sel.empty() && sel.find('\n') != std::string::npos))
  {
    return false;
  }

  const std::string text = (!sel.empty() ? sel : GetCurLine().ToStdString());

  if (mode[LINK_OPEN_MIME])
  {
    if (const path path(m_link->get_path(
          text,
          data::control().line(link::LINE_OPEN_URL),
          this));
        !path.string().empty())
    {
      if (!mode[LINK_CHECK])
      {
        browser(path.string());
      }

      return true;
    }
    else if (const wex::path mime(m_link->get_path(
               text,
               data::control().line(link::LINE_OPEN_MIME),
               this));
             !mime.string().empty())
    {
      if (!mode[LINK_CHECK])
      {
        return mime.open_mime();
      }

      return true;
    }
  }

  if (mode[LINK_OPEN])
  {
    data::control data;

    if (const wex::path path(m_link->get_path(text, data, this));
        !path.string().empty())
    {
      if (filename != nullptr)
      {
        *filename = path.fullname();
      }
      else if (!mode[LINK_CHECK])
      {
        m_frame->open_file(path, data);
      }
      else if (!mode[LINK_CHECK])
      {
        open(path, data);
      }

      return true;
    }
  }

  return false;
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

void wex::stc::on_idle(wxIdleEvent& event)
{
  event.Skip();

  if (
    is_visual() && m_file.check_sync() &&
    // the readonly flags bit of course can differ from file actual readonly
    // mode, therefore add this check
    !m_data.flags().test(data::stc::WIN_READ_ONLY) &&
    get_filename().stat().is_readonly() != GetReadOnly())
  {
    file_readonly_attribute_changed();
  }
}

void wex::stc::on_styled_text(wxStyledTextEvent& event)
{
  if (is_visual())
  {
    mark_modified(event);
  }

  event.Skip();
}

bool wex::stc::open(const path& p, const data::stc& data)
{
  m_data = data::stc(data).window(data::window().name(p.string()));

  if (get_filename() != p)
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

  m_frame->set_recent_file(p.string());

  return true;
}

void wex::stc::Paste()
{
  if (CanPaste())
  {
    factory::stc::Paste();
  }
}

bool wex::stc::position_restore()
{
  if (m_saved_selection_start != -1 && m_saved_selection_end != -1)
  {
    SetSelection(m_saved_selection_start, m_saved_selection_end);
    SetCurrentPos(m_saved_selection_start);
  }
  else if (m_saved_pos != -1)
  {
    SetSelection(m_saved_pos, m_saved_pos);
    SetCurrentPos(m_saved_pos);
  }
  else
  {
    return false;
  }

  EnsureCaretVisible();

  return true;
}

void wex::stc::position_save()
{
  m_saved_pos = GetCurrentPos();

  if (m_vi->mode().is_visual())
  {
    m_saved_selection_start = GetSelectionStart();
    m_saved_selection_end   = GetSelectionEnd();
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

  auto* frame =
    new wxPreviewFrame(preview, this, print_caption(GetName().ToStdString()));

  frame->InitializeWithModality(kind);
  frame->Show();
}

void wex::stc::properties_message(path::status_t flags)
{
  log::status() << path(get_filename(), flags);

  if (!flags[path::STAT_SYNC])
  {
    m_frame->update_statusbar(this, "PaneFileType");
    m_frame->update_statusbar(this, "PaneLexer");
    m_frame->update_statusbar(this, "PaneMode");
  }

  m_frame->update_statusbar(this, "PaneInfo");

  if (!flags[path::STAT_SYNC])
  {
    std::string title;

    if (m_vi->visual() == ex::EX)
    {
      std::string readonly;

      if (get_filename().is_readonly())
      {
        readonly = " [" + _("Readonly") + "]";
      }

      title = GetName() + readonly + " [ex]";
    }
    else
    {
      std::string readonly;

      if (GetReadOnly())
      {
        readonly = " [" + _("Readonly") + "]";
      }

      title = GetName() + readonly;
    }

    m_frame->SetTitle(
      !title.empty() ? title : wxTheApp->GetAppName().ToStdString());
  }
}

int wex::stc::replace_all(
  const std::string& find_text,
  const std::string& replace_text)
{
  int selection_from_end = 0;

  if (
    SelectionIsRectangle() ||
    get_number_of_lines(GetSelectedText().ToStdString()) > 1)
  {
    TargetFromSelection();
    selection_from_end = GetLength() - GetTargetEnd();
  }
  else
  {
    TargetWholeDocument();
  }

  int nr_replacements = 0;
  set_search_flags(-1);
  BeginUndoAction();

  while (SearchInTarget(find_text) != -1)
  {
    bool skip_replace = false;

    // Check that the target is within the rectangular selection.
    // If not just continue without replacing.
    if (SelectionIsRectangle())
    {
      const auto line      = LineFromPosition(GetTargetStart());
      const auto start_pos = GetLineSelStartPosition(line);
      const auto end_pos   = GetLineSelEndPosition(line);
      const auto length    = GetTargetEnd() - GetTargetStart();

      if (
        start_pos == wxSTC_INVALID_POSITION ||
        end_pos == wxSTC_INVALID_POSITION || GetTargetStart() < start_pos ||
        GetTargetStart() + length > end_pos)
      {
        skip_replace = true;
      }
    }

    if (!skip_replace)
    {
      if (is_hexmode())
      {
        m_hexmode.replace_target(replace_text);
      }
      else
      {
        find_replace_data::get()->is_regex() ? ReplaceTargetRE(replace_text) :
                                               ReplaceTarget(replace_text);
      }

      nr_replacements++;
    }

    SetTargetRange(GetTargetEnd(), GetLength() - selection_from_end);

    if (GetTargetStart() >= GetTargetEnd())
    {
      break;
    }
  }

  EndUndoAction();

  log::status(_("Replaced"))
    << nr_replacements << "occurrences of" << find_text;

  return nr_replacements;
}

bool wex::stc::replace_next(bool stc_find_string)
{
  return replace_next(
    find_replace_data::get()->get_find_string(),
    find_replace_data::get()->get_replace_string(),
    -1,
    stc_find_string);
}

bool wex::stc::replace_next(
  const std::string& find_text,
  const std::string& replace_text,
  int                find_flags,
  bool               stc_find_string)
{
  if (stc_find_string && !GetSelectedText().empty())
  {
    TargetFromSelection();
  }
  else
  {
    SetTargetRange(GetCurrentPos(), GetLength());
    set_search_flags(find_flags);
    if (SearchInTarget(find_text) == -1)
      return false;
  }

  if (is_hexmode())
  {
    m_hexmode.replace_target(replace_text);
  }
  else
  {
    find_replace_data::get()->is_regex() ? ReplaceTargetRE(replace_text) :
                                           ReplaceTarget(replace_text);
  }

  find(find_text, find_flags);

  return true;
}

void wex::stc::reset_margins(margin_t type)
{
  if (type[MARGIN_FOLDING])
    SetMarginWidth(m_margin_folding_number, 0);
  if (type[MARGIN_DIVIDER])
    SetMarginWidth(m_margin_divider_number, 0);
  if (type[MARGIN_LINENUMBER])
    SetMarginWidth(m_margin_line_number, 0);
  if (type[MARGIN_TEXT])
    SetMarginWidth(m_margin_text_number, 0);
}

void wex::stc::SelectNone()
{
  // The base styledtextctrl version uses scintilla, sets caret at 0.
  wxTextEntryBase::SelectNone();
}

bool wex::stc::set_indicator(const indicator& indicator, int start, int end)
{
  if (const bool loaded(lexers::get()->indicator_is_loaded(indicator));
      !loaded || start == -1 || end == -1 || end < start)
  {
    if (!loaded)
    {
      log("indicator") << indicator.number() << " not loaded";
    }
    else
    {
      log("set_indicator") << indicator.number() << start << end;
    }
    return false;
  }

  SetIndicatorCurrent(indicator.number());

  if (end - start > 0)
  {
    IndicatorFillRange(start, end - start);
  }
  else if (end - start == 0)
  {
  }

  return true;
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

bool wex::stc::show_blame(const vcs_entry* vcs)
{
  if (!vcs->get_blame().use())
  {
    return false;
  }

  if (vcs->get_stdout().empty())
  {
    log::debug("no vcs output");
    return false;
  }

  std::string prev("!@#$%");
  bool        first = true;
  int         line  = 0;

  SetWrapMode(wxSTC_WRAP_NONE);
  const item_vector& iv(m_config_items);
  const int          margin_blame(iv.find<int>(_("stc.margin.Text")));

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         vcs->get_stdout(),
         boost::char_separator<char>("\r\n")))
  {
    if (const auto& [r, bl, t, l] = vcs->get_blame().get(it); bl != prev && r)
    {
      if (first)
      {
        const int w(std::max(
          config(_("stc.Default font"))
              .get(wxFont(
                12,
                wxFONTFAMILY_DEFAULT,
                wxFONTSTYLE_NORMAL,
                wxFONTWEIGHT_NORMAL))
              .GetPixelSize()
              .GetWidth() +
            1,
          5));

        SetMarginWidth(
          m_margin_text_number,
          margin_blame == -1 ? bl.size() * w : margin_blame);

        first = false;
      }

      const int real_line(
        is_visual() ? l : l - get_current_line() + GetLineCount() - 2);

      lexers::get()->apply_margin_text_style(
        this,
        real_line >= 0 ? real_line : line,
        t,
        bl);
      prev = bl;
    }
    else
    {
      lexers::get()->apply_margin_text_style(
        this,
        l >= 0 ? l : line,
        r ? t : lexers::margin_style_t::OTHER);
    }

    line++;
  }

  return true;
}

void wex::stc::show_line_numbers(bool show)
{
  const item_vector& iv(m_config_items);

  SetMarginWidth(
    m_margin_line_number,
    show ? iv.find<int>(_("stc.margin.Line number")) : 0);
}

void wex::stc::sync(bool start)
{
  start ? Bind(wxEVT_IDLE, &stc::on_idle, this) :
          (void)Unbind(wxEVT_IDLE, &stc::on_idle, this);
}

void wex::stc::Undo()
{
  factory::stc::Undo();
  m_hexmode.undo();
}

void wex::stc::use_modification_markers(bool use)
{
  use ? Bind(wxEVT_STC_MODIFIED, &stc::on_styled_text, this) :
        (void)Unbind(wxEVT_STC_MODIFIED, &stc::on_styled_text, this);
}

bool wex::stc::vi_command(const std::string& command)
{
  return m_vi->command(command);
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
    if (m_vi->visual() != ex::VISUAL)
    {
      std::stringstream info;

      if (!get_filename().string().empty())
      {
        info << get_filename().string();
      }

      log::info("enter visual mode") << on << info;
    }

    m_vi->use(ex::VISUAL); // needed in do_file_load
    m_file.close();
    m_file.use_stream(false);
    m_file.file_load(get_filename());
  }
  else
  {
    m_vi->use(ex::EX);
  }

  config_get();

  m_frame->show_ex_bar(!on ? frame::SHOW_BAR : frame::HIDE_BAR_FOCUS_STC, this);
}
