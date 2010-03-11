/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class wxExStyledTextCtrl
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/tokenzr.h>
#include <wx/extension/stc.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI

const int SCI_ADDTEXT = 2001;
const int SCI_APPENDTEXT = 2282;

BEGIN_EVENT_TABLE(wxExStyledTextCtrl, wxStyledTextCtrl)
  EVT_CHAR(wxExStyledTextCtrl::OnChar)
  EVT_KEY_DOWN(wxExStyledTextCtrl::OnKeyDown)
  EVT_MENU(wxID_DELETE, wxExStyledTextCtrl::OnCommand)
  EVT_MENU(wxID_JUMP_TO, wxExStyledTextCtrl::OnCommand)
  EVT_MENU(wxID_SELECTALL, wxExStyledTextCtrl::OnCommand)
  EVT_MENU(wxID_SORT_ASCENDING, wxExStyledTextCtrl::OnCommand)
  EVT_MENU(wxID_SORT_DESCENDING, wxExStyledTextCtrl::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExStyledTextCtrl::OnCommand)
  EVT_MENU_RANGE(wxID_UNDO, wxID_REDO, wxExStyledTextCtrl::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, wxExStyledTextCtrl::OnCommand)
  EVT_RIGHT_UP(wxExStyledTextCtrl::OnMouse)
  EVT_STC_DWELLEND(wxID_ANY, wxExStyledTextCtrl::OnStyledText)
//  EVT_STC_DWELLSTART(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_CHARADDED(wxID_ANY, wxExStyledTextCtrl::OnStyledText)
  EVT_STC_MACRORECORD(wxID_ANY, wxExStyledTextCtrl::OnStyledText)
  EVT_STC_MARGINCLICK(wxID_ANY, wxExStyledTextCtrl::OnStyledText)
END_EVENT_TABLE()

std::vector <wxString> wxExStyledTextCtrl::m_Macro;

wxExStyledTextCtrl::wxExStyledTextCtrl() 
  : wxStyledTextCtrl()
  , m_GotoLineNumber(-1)
  , m_MacroIsRecording(false)
  , m_MarginDividerNumber(1)
  , m_MarginFoldingNumber(2)
  , m_MarginLineNumber(0)
  , m_vi(NULL)
{
}

wxExStyledTextCtrl::wxExStyledTextCtrl(wxWindow *parent, 
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size, 
  long style)
  : wxStyledTextCtrl(parent, id , pos, size, style)
  , m_MenuFlags(menu_flags)
  , m_MacroIsRecording(false)
  , m_GotoLineNumber(1)
  , m_MarginDividerNumber(1)
  , m_MarginFoldingNumber(2)
  , m_MarginLineNumber(0)
  , m_viMode(false)
  , m_vi(new wxExVi(this))
{
#ifdef __WXMSW__
  SetEOLMode(wxSTC_EOL_CRLF);
#else
  SetEOLMode(wxSTC_EOL_LF);
#endif

  SetBackSpaceUnIndents(true);
  SetMouseDwellTime(1000);
  SetMarginType(m_MarginLineNumber, wxSTC_MARGIN_NUMBER);
  SetMarginType(m_MarginDividerNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_MarginFoldingNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginMask(m_MarginFoldingNumber, wxSTC_MASK_FOLDERS);
  SetMarginSensitive(m_MarginFoldingNumber, true);

  UsePopUp(false); // we have our own

  const int accels = 15; // take max number of entries
  wxAcceleratorEntry entries[accels];

  int i = 0;

  entries[i++].Set(wxACCEL_CTRL, (int)'Z', wxID_UNDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'Y', wxID_REDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'D', ID_EDIT_HEX_DEC_CALLTIP);
  entries[i++].Set(wxACCEL_CTRL, (int)'H', ID_EDIT_CONTROL_CHAR);
  entries[i++].Set(wxACCEL_CTRL, (int)'M', ID_EDIT_MACRO_PLAYBACK);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F7, wxID_SORT_ASCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F8, wxID_SORT_DESCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F9, ID_EDIT_FOLD_ALL);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F10, ID_EDIT_UNFOLD_ALL);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F11, ID_EDIT_UPPERCASE);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F12, ID_EDIT_LOWERCASE);
  entries[i++].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[i++].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[i++].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);

  wxAcceleratorTable accel(i, entries);
  SetAcceleratorTable(accel);
}

wxExStyledTextCtrl::~wxExStyledTextCtrl()
{
  delete m_vi;
}

void wxExStyledTextCtrl::AddAsciiTable()
{
  // Do not show an edge, eol or whitespace for ascii table.
  SetEdgeMode(wxSTC_EDGE_NONE);
  SetViewEOL(false);
  SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  // And override tab width.
  SetTabWidth(10);

  for (int i = 1; i <= 255; i++)
  {
    AddText(wxString::Format("%d\t%c", i, (wxUniChar)i));
    AddText((i % 5 == 0) ? GetEOL(): "\t");
  }

  EmptyUndoBuffer();
  SetSavePoint();
}

void wxExStyledTextCtrl::AppendTextForced(const wxString& text, bool withTimestamp)
{
  const bool pos_at_end = (GetCurrentPos() == GetTextLength());
  const bool readonly = GetReadOnly();

  if (readonly)
  {
    SetReadOnly(false);
  }

  if (withTimestamp)
  {
    const wxString now = wxDateTime::Now().Format();
    AppendText(now + " " + text + GetEOL());
  }
  else
  {
    // No GetEOL, that is only added with timestamps.
    AppendText(text);
  }

  SetSavePoint();

  if (readonly)
  {
    SetReadOnly(true);
  }

  if (pos_at_end)
  {
    DocumentEnd();
  }
}

void wxExStyledTextCtrl::BuildPopupMenu(wxExMenu& menu)
{
  if (m_MenuFlags & STC_MENU_FIND && GetTextLength() > 0)
  {
    menu.AppendSeparator();
    menu.Append(wxID_FIND);
  }

  if (!GetReadOnly())
  {
    if (m_MenuFlags & STC_MENU_REPLACE && GetTextLength() > 0)
    {
      menu.Append(wxID_REPLACE);
    }
  }

  menu.AppendSeparator();
  menu.AppendEdit();

  if (!GetReadOnly())
  {
    if (!GetSelectedText().empty())
    {
      wxExMenu* menuSelection = menuSelection = new wxExMenu(menu);
      menuSelection->Append(ID_EDIT_UPPERCASE, _("&Uppercase\tF11"));
      menuSelection->Append(ID_EDIT_LOWERCASE, _("&Lowercase\tF12"));

      if (wxExGetNumberOfLines(GetSelectedText()) > 1)
      {
        wxExMenu* menuSort = new wxExMenu(menu);
        menuSort->Append(wxID_SORT_ASCENDING);
        menuSort->Append(wxID_SORT_DESCENDING);
        menuSelection->AppendSeparator();
        menuSelection->AppendSubMenu(menuSort, _("&Sort"));
      }

      menu.AppendSeparator();
      menu.AppendSubMenu(menuSelection, _("&Selection"));
    }
    else
    {
      if (m_MenuFlags & STC_MENU_INSERT)
      {
        menu.AppendSeparator();
        menu.Append(ID_EDIT_INSERT_SEQUENCE, wxExEllipsed(_("Insert Sequence")));
      }
    }
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.AppendSeparator();
    if (CanUndo()) menu.Append(wxID_UNDO);
    if (CanRedo()) menu.Append(wxID_REDO);
  }
}

void wxExStyledTextCtrl::ClearDocument()
{
  SetReadOnly(false);
  ClearAll();
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(wxEmptyString, "PaneLines");
#endif
  EmptyUndoBuffer();
  SetSavePoint();
}

void wxExStyledTextCtrl::Colourise()
{
  m_Lexer.ApplyKeywords(this);
  SetGlobalStyles();
  wxExLexers::Get()->ApplyProperties(this);
  wxExLexers::Get()->ApplyMarkers(this);
  m_Lexer.ApplyProperties(this);
  SetFolding();
  m_Lexer.Colourise(this);
}

void wxExStyledTextCtrl::ControlCharDialog(const wxString& caption)
{
  if (GetSelectedText().length() > 1)
  {
    // Do nothing
    return;
  }

  if (GetReadOnly())
  {
    if (GetSelectedText().length() == 1)
    {
      const wxUniChar value = GetSelectedText().GetChar(0);
      wxMessageBox(wxString::Format("hex: %x dec: %d", value, value), _("Control Character"));
    }

    return;
  }

  static long value = ' '; // don't use 0 as default as NULL is not handled

  if (GetSelectedText().length() == 1)
  {
    value = GetSelectedText().GetChar(0);
  }

  long new_value;
  if ((new_value = wxGetNumberFromUser(_("Input") + " 0 - 255:",
    wxEmptyString,
    caption,
    value,
    0,
    255,
    this)) < 0)
  {
    return;
  }

  if (GetSelectedText().length() == 1)
  {
    if (value != new_value)
    {
      ReplaceSelection(wxString::Format("%c", (wxUniChar)new_value));
    }

    SetSelection(GetCurrentPos(), GetCurrentPos() + 1);
  }
  else
  {
    char buffer[2];
    buffer[0] = (char)new_value;

    // README: The stc.h equivalents AddText, AddTextRaw, InsertText,
    // InsertTextRaw do not add the length.
    // To be able to add NULLs this is the only way.
    SendMsg(SCI_ADDTEXT, 1, (wxIntPtr)buffer);
  }
}

bool wxExStyledTextCtrl::FindNext(bool find_next)
{
  return FindNext(
    wxExFindReplaceData::Get()->GetFindString(),
    FindReplaceDataFlags(),
    find_next);
}

bool wxExStyledTextCtrl::FindNext(
  const wxString& text, 
  int search_flags,
  bool find_next)
{
  if (text.empty())
  {
    return false;
  }

  static bool recursive = false;
  static int start_pos, end_pos;

  if (find_next)
  {
    if (recursive) start_pos = 0;
    else
    {
      start_pos = GetCurrentPos();
      end_pos = GetTextLength();
    }
  }
  else
  {
    if (recursive) start_pos = GetTextLength();
    else
    {
      start_pos = GetCurrentPos();
      if (GetSelectionStart() != -1)
        start_pos = GetSelectionStart();

      end_pos = 0;
    }
  }

  SetTargetStart(start_pos);
  SetTargetEnd(end_pos);
  SetSearchFlags(search_flags);

  if (SearchInTarget(text) < 0)
  {
    wxExFindResult(text, find_next, recursive);
    
    if (!recursive)
    {
      recursive = true;
      FindNext(text, search_flags, find_next);
      recursive = false;
    }
    
    return false;
  }
  else
  {
    recursive = false;

    if (GetTargetStart() != GetTargetEnd())
    {
      SetSelection(GetTargetStart(), GetTargetEnd());
      EnsureVisible(LineFromPosition(GetTargetStart()));
      EnsureCaretVisible();
    }
    else
    {
      wxFAIL;
      return false;
    }

    return true;
  }
}

int wxExStyledTextCtrl::FindReplaceDataFlags() const
{
  const wxExFindReplaceData* frd = wxExFindReplaceData::Get();

  int flags = 0;

  if (frd->UseRegularExpression())  flags |= wxSTC_FIND_REGEXP;
  if (frd->MatchWord()) flags |= wxSTC_FIND_WHOLEWORD;
  if (frd->MatchCase()) flags |= wxSTC_FIND_MATCHCASE;

  return flags;
}

void wxExStyledTextCtrl::FoldAll()
{
  if (GetProperty("fold") != "1") return;

  const int current_line = GetCurrentLine();

  int line = 0;
  while (line < GetLineCount())
  {
    const int level = GetFoldLevel(line);
    const int last_child_line = GetLastChild(line, level);

    if (last_child_line > line)
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

const wxString wxExStyledTextCtrl::GetEOL() const
{
  switch (GetEOLMode())
  {
  case wxSTC_EOL_CR: return "\r"; break;
  case wxSTC_EOL_CRLF: return "\r\n"; break;
  case wxSTC_EOL_LF: return "\n"; break;
  default: wxFAIL; break;
  }

  return "\r\n";
}

int wxExStyledTextCtrl::GetLineNumberAtCurrentPos() const
{
  // This method is used by LinkOpen.
  // So, if no line number present return 0, otherwise link open jumps to last line.
  const int pos = GetCurrentPos();
  const int line_no = LineFromPosition(pos);

  // Cannot use GetLine, as that includes EOF, and then the ToLong does not
  // return correct number.
  const wxString text = const_cast< wxExStyledTextCtrl * >( this )->GetTextRange(
    PositionFromLine(line_no), 
    GetLineEndPosition(line_no));

  return wxExGetLineNumberFromText(text);
}

const wxString wxExStyledTextCtrl::GetSearchText() const
{
  const wxString selection = const_cast< wxExStyledTextCtrl * >( this )->GetSelectedText();

  if (!selection.empty() && wxExGetNumberOfLines(selection) == 1)
  {
    wxExFindReplaceData::Get()->SetFindString(selection);
  }

  return wxExFindReplaceData::Get()->GetFindString();
}

const wxString wxExStyledTextCtrl::GetTextAtCurrentPos() const
{
  const wxString sel = const_cast< wxExStyledTextCtrl * >( this )->GetSelectedText();

  if (!sel.empty())
  {
    if (wxExGetNumberOfLines(sel) > 1)
    {
      // wxPathList cannot handle links over several lines.
      return wxEmptyString;
    }

    return sel;
  }
  else
  {
    const int pos = GetCurrentPos();
    const int line_no = LineFromPosition(pos);
    const wxString text = GetLine(line_no);

    // Better first try to find "...", then <...>, as in next example.
    // <A HREF="http://www.scintilla.org">scintilla</A> component.

    // So, first get text between " signs.
    size_t pos_char1 = text.find("\"");
    size_t pos_char2 = text.rfind("\"");

    // If that did not succeed, then get text between < and >.
    if (pos_char1 == wxString::npos || pos_char2 == wxString::npos || pos_char2 <= pos_char1)
    {
      pos_char1 = text.find("<");
      pos_char2 = text.rfind(">");
    }

    // If that did not succeed, then get text between : and : (in .po files).
    if (pos_char1 == wxString::npos || pos_char2 == wxString::npos || pos_char2 <= pos_char1)
    {
      pos_char1 = text.find(": ");
      pos_char2 = text.rfind(":");
    }

    // If that did not succeed, then get text between ' and '.
    if (pos_char1 == wxString::npos || pos_char2 == wxString::npos || pos_char2 <= pos_char1)
    {
      pos_char1 = text.find("'");
      pos_char2 = text.rfind("'");
    }

    // If we did not find anything.
    if (pos_char1 == wxString::npos || pos_char2 == wxString::npos || pos_char2 <= pos_char1)
    {
      return wxEmptyString;
    }

    // Okay, get everything inbetween.
    const wxString match = text.substr(pos_char1 + 1, pos_char2 - pos_char1 - 1);

    // And make sure we skip white space.
    return match.Strip(wxString::both);
  }
}

const wxString wxExStyledTextCtrl::GetWordAtPos(int pos) const
{
  const int word_start = 
    const_cast< wxExStyledTextCtrl * >( this )->WordStartPosition(pos, true);
  const int word_end = 
    const_cast< wxExStyledTextCtrl * >( this )->WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const wxString word = 
      const_cast< wxExStyledTextCtrl * >( this )->GetTextRange(word_start, word_start + 1);

    if (!isspace(word[0]))
    {
      return word;
    }
    else
    {
      return wxEmptyString;
    }
  }
  else
  {
    const wxString word = 
      const_cast< wxExStyledTextCtrl * >( this )->GetTextRange(word_start, word_end);

    return word;
  }
}

bool wxExStyledTextCtrl::GotoDialog(const wxString& caption)
{
  wxASSERT(m_GotoLineNumber <= GetLineCount() && m_GotoLineNumber > 0);

  long val;
  if ((val = wxGetNumberFromUser(
    _("Input") + wxString::Format(" 1 - %d:", GetLineCount()),
    wxEmptyString,
    caption,
    m_GotoLineNumber, // initial value
    1,
    GetLineCount())) < 0)
  {
    return false;
  }

  GotoLineAndSelect(val, wxEmptyString);

  return true;
}

void wxExStyledTextCtrl::GotoLineAndSelect(
  int line_number, 
  const wxString& text)
{
  // line_number and m_GotoLineNumber start with 1 and is allowed to be equal to number of lines.
  // Internally GotoLine starts with 0, therefore line_number - 1 is used afterwards.
  wxASSERT(line_number <= GetLineCount() && line_number > 0);

  GotoLine(line_number - 1);
  EnsureVisible(line_number - 1);

  m_GotoLineNumber = line_number;

  const int start_pos = PositionFromLine(line_number - 1);
  const int end_pos = GetLineEndPosition(line_number - 1);

  SetTargetStart(start_pos);
  SetTargetEnd(end_pos);

  if (!text.empty())
  {
    SetSearchFlags(FindReplaceDataFlags());

    if (SearchInTarget(text) < 0)
    {
      bool recursive = true;
      wxExFindResult(text, true, recursive);
      return;
    }
  }

  SetSelection(GetTargetStart(), GetTargetEnd());
}

void wxExStyledTextCtrl::HexDecCalltip(int pos)
{
  if (CallTipActive())
  {
    CallTipCancel();
  }

  wxString word;

  if (!GetSelectedText().empty())
  {
    word = GetSelectedText();
  }
  else
  {
    word = GetWordAtPos(pos);
  }

  if (word.empty()) return;

  const wxUniChar c = word.GetChar(0);

  if (c < 32 || c > 125)
  {
    const wxString text(wxString::Format("hex: %x dec: %d", c, c));
    CallTipShow(pos, text);
    wxExClipboardAdd(text);
    return;
  }

  long base10_val, base16_val;
  const bool base10_ok = word.ToLong(&base10_val);
  const bool base16_ok = word.ToLong(&base16_val, 16);

  if (base10_ok || base16_ok)
  {
    wxString text;

    if      ( base10_ok && !base16_ok) 
      text = wxString::Format("hex: %lx", base10_val);
    else if (!base10_ok &&  base16_ok) 
      text = wxString::Format("dec: %ld", base16_val);
    else if ( base10_ok &&  base16_ok) 
      text = wxString::Format("hex: %lx dec: %ld", base10_val, base16_val);

    CallTipShow(pos, text);
    wxExClipboardAdd(text);
  }
}

bool wxExStyledTextCtrl::IsTargetRE(const wxString& target) const
{
  return 
    target.Contains("\\1") ||
    target.Contains("\\2") ||
    target.Contains("\\3") ||
    target.Contains("\\4") ||
    target.Contains("\\5") ||
    target.Contains("\\6") ||
    target.Contains("\\7") ||
    target.Contains("\\8") ||
    target.Contains("\\9");
}

void wxExStyledTextCtrl::LexerDialog(const wxString& caption)
{
  wxString lexer = m_Lexer.GetScintillaLexer();

  if (wxExLexers::Get()->ShowDialog(this, lexer, caption))
  {
    SetLexer(lexer);
  }
}

void wxExStyledTextCtrl::MacroPlayback()
{
  wxASSERT(MacroIsRecorded());

  for (
    std::vector<wxString>::const_iterator it = m_Macro.begin();
    it != m_Macro.end();
    ++it)
  {
    int msg, wp;
    char c = ' ';
    sscanf((*it).c_str(), "%d %d %c", &msg, &wp, &c);
    char txt[2];
    txt[0] = c;
    txt[1] = '\0';

    SendMsg(msg, wp, (wxIntPtr)txt);
  }

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Macro played back"));
#endif
}

void wxExStyledTextCtrl::OnChar(wxKeyEvent& event)
{
  bool skip = true;

  if (m_viMode)
  {
    // Let vi handle all keys.
    skip = m_vi->OnChar(event);
  }

  if (skip && 
       GetReadOnly() && 
       wxIsalnum(event.GetUnicodeKey()))
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Document is readonly"));
#endif
    return;
  }

  if (skip)
  {
    event.Skip();
  }
}

void wxExStyledTextCtrl::OnCommand(wxCommandEvent& command)
{
  switch (command.GetId())
  {
  case wxID_COPY: Copy(); break;
  case wxID_CUT: Cut(); break;
  case wxID_DELETE: if (!GetReadOnly()) Clear(); break;
  case wxID_JUMP_TO: GotoDialog(); break;
  case wxID_PASTE: Paste(); break;
  case wxID_SELECTALL: SelectAll(); break;
  case wxID_UNDO: Undo(); break;
  case wxID_REDO: Redo(); break;
  case wxID_SORT_ASCENDING: SortSelectionDialog(true); break;
  case wxID_SORT_DESCENDING: SortSelectionDialog(false); break;

  case ID_EDIT_CONTROL_CHAR:
    ControlCharDialog();
  break;
  case ID_EDIT_HEX_DEC_CALLTIP:
    HexDecCalltip(GetCurrentPos());
  break;

  case ID_EDIT_FOLD_ALL: FoldAll(); break;
  case ID_EDIT_UNFOLD_ALL:
    for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);
  break;
  case ID_EDIT_TOGGLE_FOLD:
  {
    const int level = GetFoldLevel(GetCurrentLine());
    const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetCurrentLine(): GetFoldParent(GetCurrentLine());
    ToggleFold(line_to_fold);
  }
  break;

  case ID_EDIT_MACRO_PLAYBACK: MacroPlayback(); break;
  case ID_EDIT_MACRO_START_RECORD: StartRecord(); break;
  case ID_EDIT_MACRO_STOP_RECORD: StopRecord(); break;

  case ID_EDIT_INSERT_SEQUENCE: SequenceDialog(); break;
  case ID_EDIT_LOWERCASE: LowerCase(); break;
  case ID_EDIT_UPPERCASE: UpperCase(); break;
  default: wxFAIL; break;
  }
}

void wxExStyledTextCtrl::OnKeyDown(wxKeyEvent& event)
{
  if ( !m_viMode ||
       (m_viMode && m_vi->OnKeyDown(event)))
  {
    if (event.GetKeyCode() == WXK_RETURN)
    {
      if (!SmartIndentation())
      {
        event.Skip();
      }
    }
    else
    {
      event.Skip();
    }
  }
}

void wxExStyledTextCtrl::OnMouse(wxMouseEvent& event)
{
  if (event.RightUp())
  {
    if (m_MenuFlags == 0)
    {
      event.Skip();
    }
    else
    {
      int style = 0; // otherwise CAN_PASTE already on
      if (GetReadOnly()) style |= wxExMenu::MENU_IS_READ_ONLY;
      if (!GetSelectedText().empty()) style |= wxExMenu::MENU_IS_SELECTED;
      if (GetTextLength() == 0) style |= wxExMenu::MENU_IS_EMPTY;
      if (CanPaste()) style |= wxExMenu::MENU_CAN_PASTE;

      wxExMenu menu(style);
      BuildPopupMenu(menu);
      PopupMenu(&menu);
    }
  }
  else
  {
    wxFAIL;
  }

  wxCommandEvent focusevent(wxEVT_COMMAND_MENU_SELECTED, ID_FOCUS_STC);
  focusevent.SetEventObject(this);
  wxPostEvent(wxTheApp->GetTopWindow(), focusevent);
}

void wxExStyledTextCtrl::OnStyledText(wxStyledTextEvent& event)
{
  if (event.GetEventType() == wxEVT_STC_DWELLEND)
  {
    if (CallTipActive())
    {
      CallTipCancel();
    }
  }
  else if (event.GetEventType() == wxEVT_STC_MACRORECORD)
  {
    wxString msg = wxString::Format("%d %d ", event.GetMessage(), event.GetWParam());

    if (event.GetLParam() != 0)
    {
      char* txt = (char *)(wxIntPtr)event.GetLParam();
      msg += txt;
    }
    else
    {
      msg += "0";
    }

    AddMacro(msg);
  }
  else if (event.GetEventType() == wxEVT_STC_MARGINCLICK)
  {
    if (event.GetMargin() == m_MarginFoldingNumber)
    {
      const int line = LineFromPosition(event.GetPosition());
      const int level = GetFoldLevel(line);

      if ((level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
      {
        ToggleFold(line);
      }
    }
  }
  else if (event.GetEventType() == wxEVT_STC_CHARADDED)
  {
    if (m_viMode)
    {
      m_vi->OnCharAdded(event);
    }
  }
  else
  {
    wxFAIL;
  }
}

#if wxUSE_PRINTING_ARCHITECTURE
void wxExStyledTextCtrl::Print(bool prompt)
{
  wxPrintData* data = wxExPrinting::Get()->GetHtmlPrinter()->GetPrintData();
  wxExPrinting::Get()->GetPrinter()->GetPrintDialogData().SetPrintData(*data);
  wxExPrinting::Get()->GetPrinter()->Print(this, new wxExPrintout(this), prompt);
}
#endif

#if wxUSE_PRINTING_ARCHITECTURE
void wxExStyledTextCtrl::PrintPreview()
{
  wxPrintPreview* preview = new wxPrintPreview(new wxExPrintout(this), new wxExPrintout(this));

  if (!preview->Ok())
  {
    delete preview;
    wxLogError("There was a problem previewing.\nPerhaps your current printer is not set correctly?");
    return;
  }

  wxPreviewFrame* frame = new wxPreviewFrame(
    preview,
    this,
    wxExPrintCaption(GetName()));

  frame->Initialize();
  frame->Show();
}
#endif

void wxExStyledTextCtrl::ReplaceAll(
  const wxString& find_text,
  const wxString& replace_text)
{
  const wxString selection = GetSelectedText();
  int selection_from_end = 0;
  const int selstart = GetSelectionStart();
  const int selend = GetSelectionEnd();

  // We cannot use wxExGetNumberOfLines here if we have a rectangular selection.
  // So do it the other way.
  if (!selection.empty() &&
       LineFromPosition(selend) > LineFromPosition(selstart))
  {
    TargetFromSelection();
    selection_from_end = GetLength() - GetTargetEnd();
  }
  else
  {
    SetTargetStart(0);
    SetTargetEnd(GetLength());
  }

  SetSearchFlags(FindReplaceDataFlags());
  int nr_replacements = 0;

  BeginUndoAction();

  while (SearchInTarget(find_text) > 0)
  {
    const int target_start = GetTargetStart();
    int length;
    bool skip_replace = false;

    // Check that the target is within the rectangular selection.
    // If not just continue without replacing.
    if (SelectionIsRectangle() && selection_from_end != 0)
    {
      const int line = LineFromPosition(target_start);
      const int start_pos = GetLineSelStartPosition(line);
      const int end_pos = GetLineSelEndPosition(line);
      length = GetTargetEnd() - target_start;

      if (start_pos == wxSTC_INVALID_POSITION ||
          end_pos == wxSTC_INVALID_POSITION ||
          target_start < start_pos ||
          target_start + length > end_pos)
      {
        skip_replace = true;
      }
    }

    if (!skip_replace)
    {
      length = (IsTargetRE(replace_text) ?
        ReplaceTargetRE(replace_text):
        ReplaceTarget(replace_text));

      nr_replacements++;
    }

    SetTargetStart(target_start + length);
    SetTargetEnd(GetLength() - selection_from_end);
  }

  EndUndoAction();

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(wxString::Format(_("Replaced: %d occurrences of: %s"),
    nr_replacements, find_text.c_str()));
#endif
}

void wxExStyledTextCtrl::ReplaceNext(bool find_next)
{
  return ReplaceNext(
    wxExFindReplaceData::Get()->GetFindString(),
    wxExFindReplaceData::Get()->GetReplaceString(),
    FindReplaceDataFlags(),
    find_next);
}

void wxExStyledTextCtrl::ReplaceNext(
  const wxString& find_text, 
  const wxString& replace_text,
  int search_flags,
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
    if (SearchInTarget(find_text) == -1) return;
  }

  IsTargetRE(replace_text) ?
    ReplaceTargetRE(replace_text):
    ReplaceTarget(replace_text);

  FindNext(find_text, search_flags, find_next);
}
  
void wxExStyledTextCtrl::ResetMargins(bool divider_margin)
{
  SetMarginWidth(m_MarginFoldingNumber, 0);
  SetMarginWidth(m_MarginLineNumber, 0);

  if (divider_margin)
  {
    SetMarginWidth(m_MarginDividerNumber, 0);
  }
}

void wxExStyledTextCtrl::SequenceDialog()
{
  static wxString start_previous;

  const wxString start = wxGetTextFromUser(
    _("Input") + ":",
    _("Start Of Sequence"),
    start_previous,
    this);

  if (start.empty()) return;

  start_previous = start;

  static wxString end_previous = start;

  const wxString end = wxGetTextFromUser(
    _("Input") + ":",
    _("End Of Sequence"),
    end_previous,
    this);

  if (end.empty()) return;

  end_previous = end;

  if (start.length() != end.length())
  {
    wxLogMessage(_("Start and end sequence should have same length"));
    return;
  }

  long lines = 1;

  for (int pos = end.length() - 1; pos >= 0; pos--)
  {
    lines *= abs(end[pos] - start[pos]) + 1;
  }

  if (wxMessageBox(wxString::Format(_("Generate %ld lines"), lines) + "?",
    _("Confirm"),
    wxOK | wxCANCEL | wxICON_QUESTION) == wxCANCEL)
  {
    return;
  }

  wxBusyCursor wait;

  wxString sequence = start;

  long actual_line = 0;

  while (sequence != end)
  {
    AddText(sequence + GetEOL());
    actual_line++;

    if (actual_line > lines)
    {
      wxFAIL;
      return;
    }

    if (start < end)
    {
      sequence.Last() = (int)sequence.Last() + 1;
    }
    else
    {
      sequence.Last() = (int)sequence.Last() - 1;
    }

    for (int pos = end.length() - 1; pos > 0; pos--)
    {
      if (start < end)
      {
        if (sequence[pos] > end[pos])
        {
          sequence[pos - 1] = (int)sequence[pos - 1] + 1;
          sequence[pos] = start[pos];
        }
      }
      else
      {
        if (sequence[pos] < end[pos])
        {
          sequence[pos - 1] = (int)sequence[pos - 1] - 1;
          sequence[pos] = start[pos];
        }
      }
    }
  }

  AddText(sequence + GetEOL());
}

void wxExStyledTextCtrl::SetFolding()
{
  if (GetProperty("fold") == "1")
  {
    SetMarginWidth(m_MarginFoldingNumber, wxConfigBase::Get()->ReadLong(_("Folding"), 16));

    SetFoldFlags(
      wxConfigBase::Get()->ReadLong(_("Fold Flags"),
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
  }
  else
  {
    SetMarginWidth(m_MarginFoldingNumber, 0);
  }
}

void wxExStyledTextCtrl::SetGlobalStyles()
{
  wxExLexers::Get()->GetDefaultStyle().Apply(this);

  StyleClearAll();

  wxExLexers::Get()->ApplyGlobalStyles(this);
  wxExLexers::Get()->ApplyIndicators(this);
}

void wxExStyledTextCtrl::SetLexer(const wxString& lexer)
{
  ClearDocumentStyle();

  // Reset all old properties. 
  // Should be before SetFileNameLexer.
  m_Lexer.ApplyResetProperties(this);

  m_Lexer = wxExLexers::Get()->FindByName(lexer);

  // Update the lexer for scintilla.
  SetLexerLanguage(m_Lexer.GetScintillaLexer());

  if (
    !m_Lexer.GetScintillaLexer().empty() &&
    // And check whether the GetLexer from scintilla has a good value.
    // Otherwise it is not known, and we better show an error.
    wxStyledTextCtrl::GetLexer() == wxSTC_LEX_NULL)
  {
    wxLogError(_("Lexer is not known") + ": " + m_Lexer.GetScintillaLexer());
  }

  Colourise();

  if (GetLineCount() > wxConfigBase::Get()->ReadLong(_("Auto fold"), -1))
  {
    FoldAll();
  }

  wxExFrame::StatusText(m_Lexer.GetScintillaLexer(), "PaneLexer");
}

void wxExStyledTextCtrl::SetLexerByText()
{
  m_Lexer = wxExLexers::Get()->FindByText(GetLine(0));
}

void wxExStyledTextCtrl::SetText(const wxString& value)
{
  ClearDocument();

  // The stc.h equivalents SetText, AddText, AddTextRaw, InsertText, InsertTextRaw do not add the length.
  // So for text with nulls this is the only way for opening.
  SendMsg(SCI_ADDTEXT, value.length(), (wxIntPtr)(const char *)value.c_str());

  DocumentStart();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

bool wxExStyledTextCtrl::SmartIndentation()
{
  // At this moment a newline has been given (but not yet processed).
  const wxString line = GetLine(GetCurrentLine());

  if (line.empty() || line == GetEOL())
  {
    return false;
  }

  // We check for a tab or space at the begin of this line,
  // and copy all these characters to the new line.
  // Using isspace is not okay, as that copies the CR and LF too, these
  // are already copied.
  int i = 0;
  if (line[i] == wxUniChar('\t') || line[i] == wxUniChar(' '))
  {
    InsertText(GetCurrentPos(), GetEOL());
    GotoLine(GetCurrentLine() + 1);

    while (line[i] == wxUniChar('\t') || line[i] == wxUniChar(' '))
    {
      InsertText(GetCurrentPos(), line[i]);
      GotoPos(GetCurrentPos() + 1);
      i++;
    }

    return true;
  }
  else
  {
    return false;
  }
}

void wxExStyledTextCtrl::SortSelectionDialog(bool sort_ascending, const wxString& caption)
{
  long val;
  if ((val = wxGetNumberFromUser(_("Input") + ":",
    wxEmptyString,
    caption,
    GetCurrentPos() + 1 - PositionFromLine(GetCurrentLine()),
    1,
    GetLineEndPosition(GetCurrentLine()),
    this)) <= 0)
  {
    return;
  }

  wxBusyCursor wait;

  const int start_line = LineFromPosition(GetSelectionStart());
  const int start_pos = PositionFromLine(start_line);
  SetSelection(start_pos, PositionFromLine(LineFromPosition(GetSelectionEnd())));

  // Empty lines are not kept after sorting, as they are used as separator.
  wxStringTokenizer tkz(GetSelectedText(), GetEOL());
  std::multimap<wxString, wxString> mm;
  while (tkz.HasMoreTokens())
  {
    const wxString line = tkz.GetNextToken() + GetEOL();

    // Use an empty key if line is to short.
    wxString key;

    if (val - 1 < (long)line.length())
    {
      key = line.substr(val - 1);
    }

    mm.insert(std::make_pair(key, line));
  }

  // The multimap is already sorted, just iterate to get all lines back.
  wxString text;
  if (sort_ascending)
  {
    for (
      std::multimap<wxString, wxString>::const_iterator it = mm.begin();
      it != mm.end();
      ++it)
    {
      text += it->second;
    }
  }
  else
  {
    for (
      std::multimap<wxString, wxString>::reverse_iterator it = mm.rbegin();
      it != mm.rend();
      ++it)
    {
      text += it->second;
    }
  }

  ReplaceSelection(text);

  // Set selection back, without removed empty lines.
  SetSelection(start_pos, GetLineEndPosition(start_line + mm.size()));
}

void wxExStyledTextCtrl::StartRecord()
{
  wxASSERT(!m_MacroIsRecording);

  m_MacroIsRecording = true;

  m_Macro.clear();

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Macro recording"));
#endif

  wxStyledTextCtrl::StartRecord();
}

void wxExStyledTextCtrl::StopRecord()
{
  wxASSERT(m_MacroIsRecording);

  m_MacroIsRecording = false;

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Macro is recorded"));
#endif

  wxStyledTextCtrl::StopRecord();
}

#endif // wxUSE_GUI
