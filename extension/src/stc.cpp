////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/app.h>
#include <wx/defs.h>
#include <wx/settings.h>
#include <wex/stc.h>
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
    HexMode() ? m_HexMode.AppendText(text): SetText(text);
    GuessType();
  }
  
  m_Data.Inject();
}

wex::stc::stc(const path& filename, const stc_data& data)
  : wxStyledTextCtrl(
      data.Window().Parent(),
      data.Window().Id(), 
      data.Window().Pos(), 
      data.Window().Size(), 
      data.Window().Style(), 
      data.Window().Name())
  , m_Data(this, data)
  , m_AutoComplete(this)
  , m_vi(this)
  , m_File(this, data.Window().Name())
  , m_Link(this)
  , m_HexMode(hexmode(this))
  , m_Frame(dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow()))
  , m_Lexer(this)
{
  if (config("AllowSync").get(true)) Sync();
  
  if (!lexers::Get()->get().empty())
  {
    m_DefaultFont = config(_("Default font")).get(
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

  SetMarginType(m_MarginLineNumber, wxSTC_MARGIN_NUMBER);
  SetMarginType(m_MarginDividerNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_MarginFoldingNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_MarginTextNumber, wxSTC_MARGIN_TEXT);

  SetMarginMask(m_MarginFoldingNumber, wxSTC_MASK_FOLDERS);

  SetMarginSensitive(m_MarginFoldingNumber, true);
  SetMarginSensitive(m_MarginTextNumber, true);

  SetMultiPaste(wxSTC_MULTIPASTE_EACH);
  SetMultipleSelection(true);
  
  if (m_Zoom == -1)
  {
    m_Zoom = GetZoom();
  }
  else
  {
    SetZoom(m_Zoom);
  }

  // we have our own popup
  UsePopUp(wxSTC_POPUP_NEVER);

  BindAll();

  if (filename.GetStat().is_ok())
  {
    Open(filename, data);
  }
  else
  {
    m_File.FileLoad(filename);
    m_Lexer.Set(filename.GetLexer(), true); // allow fold
  }

  ConfigGet();
  Fold();
}

bool wex::stc::CanCut() const
{
  return wxStyledTextCtrl::CanCut() && !GetReadOnly() && !HexMode();
}

bool wex::stc::CanPaste() const
{
  return wxStyledTextCtrl::CanPaste() && !GetReadOnly() && !HexMode();
}

void wex::stc::Clear()
{
  m_vi.GetIsActive() && GetSelectedText().empty() ?
    (void)m_vi.Command(std::string(1, WXK_DELETE)):
    wxStyledTextCtrl::Clear();
}

void wex::stc::ClearDocument(bool set_savepoint)
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
    if (m_vi.GetIsActive())
    {
      const wxCharBuffer b(GetSelectedTextRaw());
      m_vi.SetRegistersDelete(std::string(b.data(), b.length() - 1));
      m_vi.SetRegisterYank(std::string(b.data(), b.length() - 1));
    }
  
    wxStyledTextCtrl::Cut();
  }
}
  
bool wex::stc::FileReadOnlyAttributeChanged()
{
  SetReadOnly(GetFileName().IsReadOnly()); // does not return anything
  wxLogStatus(_("Readonly attribute changed"));

  return true;
}

void wex::stc::Fold(bool foldall)
{
  if (
     GetProperty("fold") == "1" &&
     m_Lexer.is_ok() &&
    !m_Lexer.GetScintillaLexer().empty())
  {
    SetMarginWidth(m_MarginFoldingNumber, config(_("Folding")).get(0));
    SetFoldFlags(config(_("Fold flags")).get(
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | 
      wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
        
    if (
      foldall || 
      GetLineCount() > config(_("Auto fold")).get(0))
    {
      FoldAll();
    }
  }
  else
  {
    SetMarginWidth(m_MarginFoldingNumber, 0);
  }
}
  
void wex::stc::FoldAll()
{
  if (GetProperty("fold") != "1") return;

  const auto current_line = GetCurrentLine();
  const bool xml = (m_Lexer.GetLanguage() == "xml");

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

const std::string wex::stc::GetEOL() const
{
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR: return "\r";
    case wxSTC_EOL_CRLF: return "\r\n";
    case wxSTC_EOL_LF: return "\n";
    default: wxFAIL; break;
  }

  return "\r\n";
}

// Cannot be const because of GetSelectedText (not const in 2.9.4).
const std::string wex::stc::GetFindString()
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
      find_replace_data::Get()->SetFindString(selection);
    }
  }

  return find_replace_data::Get()->GetFindString();
}


const std::string wex::stc::GetWordAtPos(int pos) const
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

void wex::stc::GuessType()
{
  // Get a small sample from this document to detect the file mode.
  const auto length = (!HexMode() ? GetTextLength(): m_HexMode.GetBuffer().size());
  const auto sample_size = (length > 255 ? 255: length);
  const std::string text((!HexMode() ? GetTextRange(0, sample_size).ToStdString(): 
    m_HexMode.GetBuffer().substr(0, sample_size)));
  const std::string text2((!HexMode() ? GetTextRange(length - sample_size, length).ToStdString(): 
    m_HexMode.GetBuffer().substr(length - sample_size, sample_size)));

  std::vector<std::string> v;  
  
  // If we have a modeline comment.
  if (
    m_vi.GetIsActive() && 
     (match("vi: *(set [a-z0-9:= ]+)", text, v) > 0 ||
      match("vi: *(set [a-z0-9:= ]+)", text2, v) > 0))
  {
    if (!m_vi.Command(":" + v[0] + "*")) // add * to indicate modelin
    {
      wxLogStatus("Could not apply vi settings");
    }
  }

  if      (text.find("\r\n") != std::string::npos) SetEOLMode(wxSTC_EOL_CRLF);
  else if (text.find("\n") != std::string::npos)   SetEOLMode(wxSTC_EOL_LF);
  else if (text.find("\r") != std::string::npos)   SetEOLMode(wxSTC_EOL_CR);
  else return; // do nothing

  frame::UpdateStatusBar(this, "PaneFileType");
}

bool wex::stc::LinkOpen()
{
  return LinkOpen(LINK_OPEN | LINK_OPEN_MIME);
}

bool wex::stc::LinkOpen(int mode, std::string* filename)
{
  const auto sel = GetSelectedText().ToStdString();

  if (sel.size() > 200 || 
    (!sel.empty() && sel.find('\n') != std::string::npos))
  {
    return false;
  }

  const std::string text = (!sel.empty() ? sel: GetCurLine().ToStdString());

  if (mode & LINK_OPEN_MIME)
  {
    const path path(m_Link.GetPath(text, 
      control_data().Line(link::LINE_OPEN_URL_AND_MIME)));
    
    if (!path.Path().string().empty()) 
    {
      if (!(mode & LINK_CHECK)) 
      {
        path.open_mime();
      }

      return true;
    }
  }

  if (mode & LINK_OPEN)
  {
    control_data data;
    
    if (const wex::path path(m_Link.GetPath(text, data));
      !path.Path().string().empty()) 
    {
      if (filename != nullptr)
      {
        *filename = path.GetFullName();
      }
      else if (m_Frame != nullptr)
      {
        m_Frame->OpenFile(path, data);
      }
      else
      {
        Open(path, data);
      }

      return true;
    }
  }
  
  return false;
}

bool wex::stc::MarkerDeleteAllChange()
{
  if (!lexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return false;
  }
  
  MarkerDeleteAll(m_MarkerChange.GetNo());
  
  return true;
}
  
void wex::stc::MarkModified(const wxStyledTextEvent& event)
{
  if (!lexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return;
  }
  
  UseModificationMarkers(false);
  
  if (const auto line = LineFromPosition(event.GetPosition());
    event.GetModificationType() & wxSTC_PERFORMED_UNDO)
  {
    if (event.GetLinesAdded() == 0)
    {
      MarkerDelete(line, m_MarkerChange.GetNo());
    }
    else
    {
      for (int i = 0; i < abs(event.GetLinesAdded()); i++)
      {
        MarkerDelete(line + 1, m_MarkerChange.GetNo());
      }
    }
      
    if (!IsModified())
    {
      MarkerDeleteAllChange();
    }
  }
  else if (
    (event.GetModificationType() & wxSTC_MOD_INSERTTEXT) ||
    (event.GetModificationType() & wxSTC_MOD_DELETETEXT))
  {
    if (event.GetLinesAdded() <= 0)
    {
      MarkerAdd(line, m_MarkerChange.GetNo());
    }
    else
    {
      for (int i = 0; i < event.GetLinesAdded(); i++)
      {
        MarkerAdd(line + i, m_MarkerChange.GetNo());
      }
    }
  }
    
  UseModificationMarkers(true);
}

void wex::stc::OnExit()
{
  if (config(_("Keep zoom")).get(false))
  {
    config("zoom").set(m_Zoom);
  }
}
  
void wex::stc::OnInit()
{
  if (config(_("Keep zoom")).get(false))
  {
    m_Zoom = config("zoom").get(-1);
  }
}
  
void wex::stc::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    m_File.CheckSync() &&
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !(m_Data.Flags() & stc_data::WIN_READ_ONLY) &&
      GetFileName().GetStat().is_readonly() != GetReadOnly())
  {
    FileReadOnlyAttributeChanged();
  }
}

void wex::stc::OnStyledText(wxStyledTextEvent& event)
{
  MarkModified(event); 
  event.Skip();
}

bool wex::stc::Open(const path& filename, const stc_data& data)
{
  if (GetFileName() != filename && !m_File.FileLoad(filename))
  {
    return false;
  }

  m_Data = stc_data(data).Window(window_data().Name(filename.Path().string()));
  m_Data.Inject();

  if (m_Frame != nullptr)
  {
    m_Frame->SetRecentFile(filename);
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

bool wex::stc::PositionRestore()
{
  if (m_vi.Mode().Visual())
  {
    SetCurrentPos(m_SavedPos);
  }
  else if (m_SavedSelectionStart != -1 && m_SavedSelectionEnd != -1)
  {
    SetSelection(m_SavedSelectionStart, m_SavedSelectionEnd);
    SetCurrentPos(m_SavedSelectionStart);
  }
  else if (m_SavedPos != -1)
  {
    SetSelection(m_SavedPos, m_SavedPos);
    SetCurrentPos(m_SavedPos);
  }
  else
  {
    return false;
  }
  
  EnsureCaretVisible();
  
  return true;
}
  
void wex::stc::PositionSave()
{
  m_SavedPos = GetCurrentPos();

  if (!m_vi.Mode().Visual())
  {
    m_SavedSelectionStart = GetSelectionStart();  
    m_SavedSelectionEnd = GetSelectionEnd();
  }
}

#if wxUSE_PRINTING_ARCHITECTURE
void wex::stc::Print(bool prompt)
{
  wxPrintData* data = printing::Get()->GetHtmlPrinter()->GetPrintData();
  printing::Get()->GetPrinter()->GetPrintDialogData().SetPrintData(*data);
  printing::Get()->GetPrinter()->Print(this, new printout(this), prompt);
}
#endif

#if wxUSE_PRINTING_ARCHITECTURE
void wex::stc::PrintPreview(wxPreviewFrameModalityKind kind)
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

void wex::stc::PropertiesMessage(long flags)
{
  log_status(GetFileName(), flags);
  
  if (flags != STAT_SYNC)
  {
    frame::UpdateStatusBar(this, "PaneFileType");
    frame::UpdateStatusBar(this, "PaneLexer");
    frame::UpdateStatusBar(this, "PaneMode");
  }
  
  frame::UpdateStatusBar(this, "PaneInfo");

  if (!(flags & STAT_SYNC) && m_Frame != nullptr)
  {
    const wxString file = GetName() + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString());
    
    m_Frame->SetTitle(!file.empty() ? file: wxTheApp->GetAppName());
  }
}

int wex::stc::ReplaceAll(
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
  SetSearchFlags(-1);
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
      if (HexMode())
      {
        m_HexMode.ReplaceTarget(replace_text);
      }
      else
      {
        find_replace_data::Get()->UseRegEx() ?
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

  wxLogStatus(_("Replaced: %d occurrences of: %s"),
    nr_replacements, find_text.c_str());

  return nr_replacements;
}

bool wex::stc::ReplaceNext(bool find_next)
{
  return ReplaceNext(
    find_replace_data::Get()->GetFindString(),
    find_replace_data::Get()->GetReplaceString(),
    -1,
    find_next);
}

bool wex::stc::ReplaceNext(
  const std::string& find_text, 
  const std::string& replace_text,
  int find_flags,
  bool find_next)
{
  if (!GetSelectedText().empty())
  {
    TargetFromSelection();
  }
  else
  {
    SetTargetStart(GetCurrentPos());
    SetTargetEnd(GetLength());
    SetSearchFlags(find_flags);
    if (SearchInTarget(find_text) == -1) return false;
  }

  if (HexMode())
  {
    m_HexMode.ReplaceTarget(replace_text);
  }
  else
  {
    find_replace_data::Get()->UseRegEx() ?
      ReplaceTargetRE(replace_text):
      ReplaceTarget(replace_text);
  }

  FindNext(find_text, find_flags, find_next);
  
  return true;
}

 
void wex::stc::ResetMargins(margin_flags flags)
{
  if (flags & MARGIN_FOLDING) SetMarginWidth(m_MarginFoldingNumber, 0);
  if (flags & MARGIN_DIVIDER) SetMarginWidth(m_MarginDividerNumber, 0);
  if (flags & MARGIN_LINENUMBER) SetMarginWidth(m_MarginLineNumber, 0);
  if (flags & MARGIN_TEXT) SetMarginWidth(m_MarginTextNumber, 0);
}

void wex::stc::SelectNone()
{
  if (SelectionIsRectangle())
  {
    // SetSelection does not work.
    CharRight();
    CharLeft();
  }
  else
  {
    // The base styledtextctrl version uses scintilla, sets caret at 0.
    SetSelection(GetCurrentPos(), GetCurrentPos());
  }
}

bool wex::stc::SetIndicator(const indicator& indicator, int start, int end)
{
  if (!lexers::Get()->IndicatorIsLoaded(indicator))
  {
    return false;
  }

  SetIndicatorCurrent(indicator.GetNo());
  IndicatorFillRange(start, end - start);
  
  return true;
}

void wex::stc::SetSearchFlags(int flags)
{
  if (flags == -1)
  {
    flags = 0;
    
    auto* frd = find_replace_data::Get();
    if (frd->UseRegEx()) flags |= wxSTC_FIND_REGEXP | wxSTC_FIND_CXX11REGEX;
    if (frd->MatchWord()) flags |= wxSTC_FIND_WHOLEWORD;
    if (frd->MatchCase()) flags |= wxSTC_FIND_MATCHCASE;
  }

  wxStyledTextCtrl::SetSearchFlags(flags);
}

void wex::stc::SetText(const std::string& value)
{
  ClearDocument();

  AddTextRaw((const char *)value.c_str(), value.length());

  DocumentStart();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

void wex::stc::ShowLineNumbers(bool show)
{
  SetMarginWidth(m_MarginLineNumber, 
    show ? config(_("Line number")).get(0): 0);
}

bool wex::stc::ShowVCS(const vcs_entry* vcs)
{
  if (vcs->GetMarginWidth() <= 0)
  {
    return false;
  }

  if (vcs->GetStdOut().empty())
  {
    return false;
  }

  int begin, end;
  bool begin_is_number, end_is_number = true;

  try
  {
    begin = std::stoi(vcs->GetPosBegin());
  }
  catch (std::exception& )
  {
    begin_is_number = false;
  }

  try
  {
    end = std::stoi(vcs->GetPosEnd());
  }
  catch (std::exception& )
  {
    end_is_number = false;
  }

  int line = 0;
  bool found = false;

  for (tokenizer tkz(vcs->GetStdOut(), "\r\n"); tkz.HasMoreTokens(); )
  {
    const auto text(tkz.GetNextToken());

    if (!begin_is_number)
    {
      begin = text.find(vcs->GetPosBegin());
    }

    if (!end_is_number)
    {
      end = text.find(vcs->GetPosEnd());
    }

    if (begin != std::string::npos && end != std::string::npos)
    {
      MarginSetText(line, text.substr(begin + 1, end - begin - 1));
      lexers::Get()->ApplyMarginTextStyle(this, line);
      found = true;
    }
 
    line++;
  }

  if (found)
  {
    SetMarginWidth(m_MarginTextNumber, vcs->GetMarginWidth());
  }

  return found;
}

void wex::stc::Sync(bool start)
{
  start ?
    Bind(wxEVT_IDLE, &stc::OnIdle, this):
    (void)Unbind(wxEVT_IDLE, &stc::OnIdle, this);
}

void wex::stc::Undo()
{
  wxStyledTextCtrl::Undo();
  m_HexMode.Undo();
}

void wex::stc::UseModificationMarkers(bool use)
{
  use ?
    Bind(wxEVT_STC_MODIFIED, &stc::OnStyledText, this):
    (void)Unbind(wxEVT_STC_MODIFIED, &stc::OnStyledText, this);
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
