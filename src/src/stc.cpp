////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/app.h>
#include <wx/defs.h>
#include <wx/settings.h>
#include <wex/stc.h>
#include <wex/blame.h>
#include <wex/config.h>
#include <wex/frd.h>
#include <wex/indicator.h>
#include <wex/lexers.h>
#include <wex/managedframe.h>
#include <wex/path.h>
#include <wex/printing.h>
#include <wex/stcdlg.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <wex/vcs.h>

wex::stc::stc(const std::string& text, const stc_data& data)
  : stc(path(), data)
{
  if (!text.empty())
  {
    is_hexmode() ? m_hexmode.append_text(text): set_text(text);
    guess_type();
  }
  
  m_data.inject();
}

wex::stc::stc(const path& p, const stc_data& data)
  : wxStyledTextCtrl(
      data.window().parent(),
      data.window().id(), 
      data.window().pos(), 
      data.window().size(), 
      data.window().style(), 
      data.window().name())
  , m_data(this, data)
  , m_auto_complete(this)
  , m_vi(this)
  , m_file(this, data.window().name())
  , m_link(this)
  , m_hexmode(hexmode(this))
  , m_frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()))
  , m_lexer(this)
{
  if (config("AllowSync").get(true)) sync();
  
  if (!lexers::get()->get_lexers().empty())
  {
    m_default_font = config(_("Default font")).get(
      wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT));
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

  config_get();

  if (p.stat().is_ok())
  {
    open(p, data);
  }
  else
  {
    m_lexer.set(p.lexer(), true);
    m_file.file_new(p);
  }
}

void wex::stc::add_text(const std::string& text)
{
  if (!GetOvertype())
  {
    Allocate(GetTextLength() + text.size());
    AddTextRaw(text.data(), text.size());
  }
  else
  {
    SetTargetStart(GetCurrentPos());
    SetTargetEnd(GetCurrentPos() + text.size());
    ReplaceTarget(text);
  }
}

void wex::stc::append_text(const std::string& text)
{
  Allocate(GetTextLength() + text.size());
  AppendTextRaw(text.data(), text.size());
}
    
#define BIGWORD(DIRECTION)                   \
  int c = GetCharAt(GetCurrentPos());        \
  int offset = strncmp((#DIRECTION), "Left", 4) == 0 ? -1: 0; \
  while (isspace(c) &&                       \
    GetCurrentPos() > 0 &&                   \
    GetCurrentPos() < GetTextLength())       \
  {                                          \
    Char##DIRECTION();                       \
    c = GetCharAt(GetCurrentPos() + offset); \
  }                                          \
  while (!isspace(c) &&                      \
    GetCurrentPos() > 0 &&                   \
    GetCurrentPos() < GetTextLength())       \
  {                                          \
    Char##DIRECTION();                       \
    c = GetCharAt(GetCurrentPos() + offset); \
  }

void wex::stc::BigWordLeft()
{
  BIGWORD( Left );
}
  
void wex::stc::BigWordLeftExtend()
{
  BIGWORD( LeftExtend );
}
  
void wex::stc::BigWordLeftRectExtend()
{
  BIGWORD( LeftRectExtend );
}
  
void wex::stc::BigWordRight()
{
  BIGWORD( Right );
}
  
void wex::stc::BigWordRightEnd()
{
  BIGWORD( Right );
}
  
void wex::stc::BigWordRightEndExtend()
{
  BIGWORD( RightExtend );
}
  
void wex::stc::BigWordRightEndRectExtend()
{
  BIGWORD( RightRectExtend );
}

void wex::stc::BigWordRightExtend()
{
  BIGWORD( RightExtend );
}

void wex::stc::BigWordRightRectExtend()
{
  BIGWORD( RightRectExtend );
}

void wex::stc::PageScrollDown()
{
  LineScroll(0, 10);
}

void wex::stc::PageScrollUp()
{
  LineScroll(0, -10);
}

bool wex::stc::CanCut() const
{
  return wxStyledTextCtrl::CanCut() && !GetReadOnly() && !is_hexmode();
}

bool wex::stc::CanPaste() const
{
  return wxStyledTextCtrl::CanPaste() && !GetReadOnly() && !is_hexmode();
}

void wex::stc::Clear()
{
  m_vi.is_active() && GetSelectedText().empty() ?
    (void)m_vi.command(std::string(1, WXK_DELETE)):
    wxStyledTextCtrl::Clear();
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
    wxStyledTextCtrl::Copy();
  }
}

void wex::stc::Cut()
{
  if (CanCut()) 
  {
    if (m_vi.is_active())
    {
      m_vi.set_registers_delete(get_selected_text());
      m_vi.set_register_yank(get_selected_text());
    }
  
    wxStyledTextCtrl::Cut();
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
  if (
     GetProperty("fold") == "1" &&
     m_lexer.is_ok() &&
    !m_lexer.scintilla_lexer().empty())
  {
    SetMarginWidth(m_margin_folding_number, config(_("Folding")).get(0));
    SetFoldFlags(config(_("Fold flags")).get(
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | 
      wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
        
    if (
      all || 
      GetLineCount() > config(_("Auto fold")).get(0))
    {
      fold_all();
    }
  }
  else
  {
    SetMarginWidth(m_margin_folding_number, 0);
  }
}
  
void wex::stc::fold_all()
{
  if (GetProperty("fold") != "1") return;

  const auto current_line = GetCurrentLine();
  const bool xml = (m_lexer.language() == "xml");

  int line = 0;
  while (line < GetLineCount())
  {
    if (const auto level = GetFoldLevel(line); xml && (
        level == wxSTC_FOLDLEVELBASE + wxSTC_FOLDLEVELHEADERFLAG))
    {
      line++;
    }
    else if (const auto last_child_line = GetLastChild(line, level);
      last_child_line > line + 1)
    {
      if (GetFoldExpanded(line)) ToggleFold(line);
      line = last_child_line + 1;
    }
    else
    {
      line++;
    }
  }

  GotoLine(current_line);
}

const std::string wex::stc::eol() const
{
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR: return "\r";
    case wxSTC_EOL_CRLF: return "\r\n";
    case wxSTC_EOL_LF: return "\n";
    default: assert(0); break;
  }

  return "\r\n";
}

// Cannot be const because of GetSelectedText (not const in 2.9.4).
const std::string wex::stc::get_find_string()
{
  if (const auto selection = GetSelectedText().ToStdString();
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
          !isalnum(selection[i]) && 
           selection[i] != ' ' && 
           selection[i] != '.' && 
           selection[i] != '-' && 
           selection[i] != '_')
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

const std::string wex::stc::get_selected_text()
{
  const wxCharBuffer b(GetSelectedTextRaw());
  return b.length() == 0 ? 
    std::string():
    std::string(b.data(), b.length() - 1);
}

const std::string wex::stc::get_word_at_pos(int pos) const
{
  const auto word_start = 
    const_cast< stc * >( this )->WordStartPosition(pos, true);
  const auto word_end = 
    const_cast< stc * >( this )->WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const std::string word = 
      const_cast< stc * >( this )->GetTextRange(word_start, word_start + 1).ToStdString();

    return !isspace(word[0]) ? word: std::string();
  }
  else
  {
    return const_cast< stc * >( this )->GetTextRange(word_start, word_end).ToStdString();
  }
}

void wex::stc::guess_type()
{
  // Get a small sample from this document to detect the file mode.
  const auto length = (!is_hexmode() ? GetTextLength(): m_hexmode.buffer().size());
  const auto sample_size = (length > 255 ? 255: length);
  const std::string text((!is_hexmode() ? GetTextRange(0, sample_size).ToStdString(): 
    m_hexmode.buffer().substr(0, sample_size)));
  const std::string text2((!is_hexmode() ? GetTextRange(length - sample_size, length).ToStdString(): 
    m_hexmode.buffer().substr(length - sample_size, sample_size)));

  std::vector<std::string> v;  
  
  // If we have a modeline comment.
  if (
    const std::string modeline("\\s+vim?:\\s*(set [a-z0-9:= ]+)");
    m_vi.is_active() && 
     (match(modeline, text, v) > 0 ||
      match(modeline, text2, v) > 0))
  {
    if (!m_vi.command(":" + v[0] + "*")) // add * to indicate modeline
    {
      log::status("Could not apply vi settings");
    }
  }

  if      (text.find("\r\n") != std::string::npos) SetEOLMode(wxSTC_EOL_CRLF);
  else if (text.find("\n") != std::string::npos)   SetEOLMode(wxSTC_EOL_LF);
  else if (text.find("\r") != std::string::npos)   SetEOLMode(wxSTC_EOL_CR);
  else return; // do nothing

  frame::update_statusbar(this, "PaneFileType");
}

bool wex::stc::link_open()
{
  return link_open(link_t().set(LINK_OPEN).set(LINK_OPEN_MIME));
}

bool wex::stc::link_open(link_t mode, std::string* filename)
{
  const auto sel = GetSelectedText().ToStdString();

  if (sel.size() > 200 || 
    (!sel.empty() && sel.find('\n') != std::string::npos))
  {
    return false;
  }

  const std::string text = (!sel.empty() ? sel: GetCurLine().ToStdString());

  if (mode[LINK_OPEN_MIME])
  {
    const path path(m_link.get_path(text, 
      control_data().line(link::LINE_OPEN_URL_AND_MIME)));
    
    if (!path.string().empty()) 
    {
      if (!mode[LINK_CHECK]) 
      {
        return path.open_mime();
      }

      return true;
    }
  }

  if (mode[LINK_OPEN])
  {
    control_data data;
    
    if (const wex::path path(m_link.get_path(text, data));
      !path.string().empty()) 
    {
      if (filename != nullptr)
      {
        *filename = path.fullname();
      }
      else if (m_frame != nullptr && !mode[LINK_CHECK])
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

void wex::stc::on_exit()
{
  if (config(_("Keep zoom")).get(false))
  {
    config("zoom").set(m_zoom);
  }
}
  
void wex::stc::on_init()
{
  if (config(_("Keep zoom")).get(false))
  {
    m_zoom = config("zoom").get(-1);
  }
}
  
void wex::stc::on_idle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    m_file.check_sync() &&
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !m_data.flags().test(stc_data::WIN_READ_ONLY) &&
    get_filename().stat().is_readonly() != GetReadOnly())
  {
    file_readonly_attribute_changed();
  }
}

void wex::stc::on_styled_text(wxStyledTextEvent& event)
{
  mark_modified(event); 
  event.Skip();
}

bool wex::stc::open(const path& p, const stc_data& data)
{
  m_data = stc_data(data).window(window_data().name(p.string()));

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

  if (m_frame != nullptr)
  {
    m_frame->set_recent_file(p.string());
  }

  return true;
}

void wex::stc::Paste()
{
  if (CanPaste())
  {
    wxStyledTextCtrl::Paste();
  }
}

bool wex::stc::position_restore()
{
  if (m_vi.mode().visual())
  {
    SetCurrentPos(m_saved_pos);
  }
  else if (m_saved_selection_start != -1 && m_saved_selection_end != -1)
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

  if (!m_vi.mode().visual())
  {
    m_saved_selection_start = GetSelectionStart();  
    m_saved_selection_end = GetSelectionEnd();
  }
}

#if wxUSE_PRINTING_ARCHITECTURE
void wex::stc::print(bool prompt)
{
  wxPrintData* data = printing::get()->get_html_printer()->GetPrintData();
  printing::get()->get_printer()->GetPrintDialogData().SetPrintData(*data);
  printing::get()->get_printer()->Print(this, new printout(this), prompt);
}
#endif

#if wxUSE_PRINTING_ARCHITECTURE
void wex::stc::print_preview(wxPreviewFrameModalityKind kind)
{
  auto* preview = new wxPrintPreview(
    new printout(this), 
    new printout(this));

  if (!preview->Ok())
  {
    delete preview;
    stc_entry_dialog("There was a problem previewing.\n"
      "Perhaps your current printer is not set correctly?").ShowModal();
    return;
  }

  auto* frame = new wxPreviewFrame(
    preview,
    this,
    print_caption(GetName().ToStdString()));

  frame->InitializeWithModality(kind);
  frame->Show();
}
#endif

void wex::stc::properties_message(path::status_t flags)
{
  log::status() << path(get_filename(), flags);
  
  if (!flags[path::STAT_SYNC])
  {
    frame::update_statusbar(this, "PaneFileType");
    frame::update_statusbar(this, "PaneLexer");
    frame::update_statusbar(this, "PaneMode");
  }
  
  frame::update_statusbar(this, "PaneInfo");

  if (!flags[path::STAT_SYNC] && m_frame != nullptr)
  {
    const wxString file = GetName() + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString());
    
    m_frame->SetTitle(!file.empty() ? file: wxTheApp->GetAppName());
  }
}

int wex::stc::replace_all(
  const std::string& find_text,
  const std::string& replace_text)
{
  int selection_from_end = 0;

  if (SelectionIsRectangle() || get_number_of_lines(GetSelectedText().ToStdString()) > 1)
  {
    TargetFromSelection();
    selection_from_end = GetLength() - GetTargetEnd();
  }
  else
  {
    SetTargetStart(0);
    SetTargetEnd(GetLength());
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
      const auto line = LineFromPosition(GetTargetStart());
      const auto start_pos = GetLineSelStartPosition(line);
      const auto end_pos = GetLineSelEndPosition(line);
      const auto length = GetTargetEnd() - GetTargetStart();

      if (start_pos == wxSTC_INVALID_POSITION ||
          end_pos == wxSTC_INVALID_POSITION ||
          GetTargetStart() < start_pos ||
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
        find_replace_data::get()->use_regex() ?
          ReplaceTargetRE(replace_text):
          ReplaceTarget(replace_text);
      }

      nr_replacements++;
    }

    SetTargetStart(GetTargetEnd());
    SetTargetEnd(GetLength() - selection_from_end);
    
    if (GetTargetStart() >= GetTargetEnd())
    {
      break;
    }
  }

  EndUndoAction();

  log::status(_("Replaced")) << nr_replacements << "occurrences of" << find_text;

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
  int find_flags,
  bool stc_find_string)
{
  if (stc_find_string && !GetSelectedText().empty())
  {
    TargetFromSelection();
  }
  else
  {
    SetTargetStart(GetCurrentPos());
    SetTargetEnd(GetLength());
    set_search_flags(find_flags);
    if (SearchInTarget(find_text) == -1) return false;
  }

  if (is_hexmode())
  {
    m_hexmode.replace_target(replace_text);
  }
  else
  {
    find_replace_data::get()->use_regex() ?
      ReplaceTargetRE(replace_text):
      ReplaceTarget(replace_text);
  }

  find_next(find_text, find_flags);
  
  return true;
}

 
void wex::stc::reset_margins(margin_t type)
{
  if (type[MARGIN_FOLDING]) SetMarginWidth(m_margin_folding_number, 0);
  if (type[MARGIN_DIVIDER]) SetMarginWidth(m_margin_divider_number, 0);
  if (type[MARGIN_LINENUMBER]) SetMarginWidth(m_margin_line_number, 0);
  if (type[MARGIN_TEXT]) SetMarginWidth(m_margin_text_number, 0);
}

void wex::stc::SelectNone()
{
  // The base styledtextctrl version uses scintilla, sets caret at 0.
  wxTextEntryBase::SelectNone();
}

bool wex::stc::set_indicator(const indicator& indicator, int start, int end)
{
  if (!lexers::get()->indicator_is_loaded(indicator))
  {
    return false;
  }

  SetIndicatorCurrent(indicator.number());
  IndicatorFillRange(start, end - start);
  
  return true;
}

void wex::stc::set_search_flags(int flags)
{
  if (flags == -1)
  {
    flags = 0;
    
    auto* frd = find_replace_data::get();
    if (frd->use_regex()) flags |= wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX;
    if (frd->match_word()) flags |= wxSTC_FIND_WHOLEWORD;
    if (frd->match_case()) flags |= wxSTC_FIND_MATCHCASE;
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
    log::verbose("no vcs output");
    return false;
  }

  std::string prev ("!@#$%");
  bool first = true;
  int line = 0;

  for (tokenizer tkz(vcs->get_stdout(), "\r\n"); tkz.has_more_tokens(); )
  {
    if (const auto [r, bl, t, l] = vcs->get_blame().get(tkz.get_next_token());
      bl != prev && r)
    {
      if (first)
      {
        SetMarginWidth(
          m_margin_text_number, 
          bl.size() * 
           (StyleGetFont(m_margin_text_number).GetPixelSize().GetWidth() + 1));
        first = false;
      }
      
      lexers::get()->apply_margin_text_style(this, l >= 0 ? l: line, t, bl);
      prev = bl;
    }
    else
    {
      lexers::get()->apply_margin_text_style(this, l >= 0 ? l: line, 
        r ? t: lexers::margin_style_t::OTHER);
    }
    
    line++;
  }

  return true;
}

void wex::stc::show_line_numbers(bool show)
{
  SetMarginWidth(m_margin_line_number, 
    show ? config(_("Line number")).get(0): 0);
}

void wex::stc::sync(bool start)
{
  start ?
    Bind(wxEVT_IDLE, &stc::on_idle, this):
    (void)Unbind(wxEVT_IDLE, &stc::on_idle, this);
}

void wex::stc::Undo()
{
  wxStyledTextCtrl::Undo();
  m_hexmode.undo();
}

void wex::stc::use_modification_markers(bool use)
{
  use ?
    Bind(wxEVT_STC_MODIFIED, &stc::on_styled_text, this):
    (void)Unbind(wxEVT_STC_MODIFIED, &stc::on_styled_text, this);
}

void wex::stc::WordLeftRectExtend() 
{
  const auto repeat = GetCurrentPos() - WordStartPosition(GetCurrentPos(), false);
  
  for (auto i = 0; i < repeat ; i++)
  {
    CharLeftRectExtend();
  }
}

void wex::stc::WordRightRectExtend() 
{
  const auto repeat = WordEndPosition(GetCurrentPos(), false) - GetCurrentPos();
  
  for (auto i = 0; i < repeat; i++)
  {
    CharRightRectExtend();
  }
}
