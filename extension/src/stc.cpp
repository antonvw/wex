////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/numdlg.h>
#include <wx/tokenzr.h>
#include <wx/extension/stc.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/indicator.h>
#include <wx/extension/lexer.h>
#include <wx/extension/lexers.h>
#include <wx/extension/link.h>
#include <wx/extension/menu.h>
#include <wx/extension/printing.h>
#include <wx/extension/stcdlg.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI

BEGIN_EVENT_TABLE(wxExSTC, wxStyledTextCtrl)
  EVT_CHAR(wxExSTC::OnChar)
  EVT_FIND(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_FIND_REPLACE(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_FIND_REPLACE_ALL(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_KEY_DOWN(wxExSTC::OnKeyDown)
  EVT_KEY_UP(wxExSTC::OnKeyUp)
  EVT_LEFT_UP(wxExSTC::OnMouse)
  EVT_MENU(ID_EDIT_OPEN_BROWSER, wxExSTC::OnCommand)
  EVT_MENU(ID_EDIT_OPEN_LINK, wxExSTC::OnCommand)
  EVT_MENU(ID_EDIT_READ, wxExSTC::OnCommand)
  EVT_MENU(wxID_DELETE, wxExSTC::OnCommand)
  EVT_MENU(wxID_FIND, wxExSTC::OnCommand)
  EVT_MENU(wxID_REPLACE, wxExSTC::OnCommand)
  EVT_MENU(wxID_JUMP_TO, wxExSTC::OnCommand)
  EVT_MENU(wxID_SELECTALL, wxExSTC::OnCommand)
  EVT_MENU(wxID_SORT_ASCENDING, wxExSTC::OnCommand)
  EVT_MENU(wxID_SORT_DESCENDING, wxExSTC::OnCommand)
  EVT_MENU(ID_EDIT_FIND_NEXT, wxExSTC::OnCommand)
  EVT_MENU(ID_EDIT_FIND_PREVIOUS, wxExSTC::OnCommand)
  EVT_MENU(ID_EDIT_SHOW_PROPERTIES, wxExSTC::OnCommand)  
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_UNDO, wxID_REDO, wxExSTC::OnCommand)
  EVT_RIGHT_UP(wxExSTC::OnMouse)
  EVT_STC_AUTOCOMP_SELECTION(wxID_ANY, wxExSTC::OnStyledText)  
  EVT_STC_CHARADDED(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_DO_DROP(wxID_ANY, wxExSTC::OnStyledText)  
  EVT_STC_DWELLEND(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MARGINCLICK(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_START_DRAG(wxID_ANY, wxExSTC::OnStyledText)
END_EVENT_TABLE()

wxExConfigDialog* wxExSTC::m_ConfigDialog = NULL;
wxExSTCEntryDialog* wxExSTC::m_EntryDialog = NULL;
int wxExSTC::m_Zoom = -1;

wxExSTC::wxExSTC(wxWindow *parent, 
  const wxString& value,
  long win_flags,
  const wxString& title,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size, 
  long style)
  : wxStyledTextCtrl(parent, id , pos, size, style, title)
  , m_Flags(win_flags)
  , m_MenuFlags(menu_flags)
  , m_Goto(1)
  , m_MarginDividerNumber(1)
  , m_MarginFoldingNumber(2)
  , m_MarginLineNumber(0)
  , m_MarkerChange(1, -1)
  , m_vi(wxExVi(this))
  , m_File(this, title)
  , m_Link(wxExLink(this))
{
  Initialize(false);

  PropertiesMessage();

  if (!value.empty())
  {
    if (HexMode())
    {
      AppendTextHexMode(value.c_str());
    }
    else
    {
      SetText(value);
    }

    GuessType();
  }
  
  if (m_Flags & STC_WIN_READ_ONLY)
  {
    SetReadOnly(true);
  }
}

wxExSTC::wxExSTC(wxWindow* parent,
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxStyledTextCtrl(parent, id, pos, size, style)
  , m_File(this)
  , m_Goto(1)
  , m_MarginDividerNumber(1)
  , m_MarginFoldingNumber(2)
  , m_MarginLineNumber(0)
  , m_MarkerChange(1, -1)
  , m_Flags(flags)
  , m_MenuFlags(menu_flags)
  , m_vi(wxExVi(this))
  , m_Link(wxExLink(this))
{
  Initialize(filename.GetStat().IsOk());
  
  if (filename.GetStat().IsOk())
  {
    Open(filename, line_number, match, col_number, flags);
  }
}

wxExSTC::wxExSTC(const wxExSTC& stc)
  : wxStyledTextCtrl(stc.GetParent(), 
      wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, stc.GetName())
  , m_Flags(stc.m_Flags)
  , m_Goto(stc.m_Goto)
  , m_MenuFlags(stc.m_MenuFlags)
  , m_MarginDividerNumber(stc.m_MarginDividerNumber)
  , m_MarginFoldingNumber(stc.m_MarginFoldingNumber)
  , m_MarginLineNumber(stc.m_MarginLineNumber)
  , m_MarkerChange(stc.m_MarkerChange)
  , m_vi(wxExVi(this)) // do not use stc.m_vi, crash
  , m_File(this, stc.m_File.GetFileName().GetFullPath())
  , m_Link(wxExLink(this))
{
  Initialize(stc.m_File.GetFileName().GetStat().IsOk());

  if (stc.m_File.GetFileName().GetStat().IsOk())
  {
    Open(stc.m_File.GetFileName(), -1, wxEmptyString, 0, GetFlags());
  }
}

void wxExSTC::AppendTextHexMode(const wxCharBuffer& buffer)
{
  wxExHexModeLine(this).AppendText(buffer);
}

bool wxExSTC::AutoIndentation(int c)
{
  bool is_nl = false;
  
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR:   is_nl = (c == '\r'); break;
    case wxSTC_EOL_CRLF: is_nl = (c== '\n'); break; // so ignore first \r
    case wxSTC_EOL_LF:   is_nl = (c== '\n'); break;
  }
  
  const int currentLine = GetCurrentLine();
  
  if (!is_nl || currentLine == 0)
  {
    return false;
  }

  // the current line has yet no indents, so use previous line
  int indent = GetLineIndentation(currentLine - 1);
  const int level = 
    (GetFoldLevel(currentLine) & wxSTC_FOLDLEVELNUMBERMASK) 
    - wxSTC_FOLDLEVELBASE;
  bool dec = false;
    
  if (level != m_FoldLevel)
  {
    if (level > m_FoldLevel)
    {
      indent += GetIndent();
    }
    else
    {
      indent -= GetIndent();
      dec = true;
    }
    
    m_FoldLevel = level;
  }
    
  if (indent == 0 && !dec) 
  {
    return false;
  }
  
  BeginUndoAction();

  SetLineIndentation(currentLine, indent);
    
  if (dec && m_AddingChars)
  {
    SetLineIndentation(currentLine - 1, indent);
  }
  
  EndUndoAction();

  GotoPos(GetLineIndentPosition(currentLine));

  return true;
}

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const wxString sel = GetSelectedText();

  if (GetCurrentLine() == 0 && wxExLexers::Get()->GetCount() > 0)
  {
    menu.Append(ID_EDIT_SHOW_PROPERTIES, _("Properties"));
  }
    
  if (m_MenuFlags & STC_MENU_OPEN_LINK)
  {
    wxString filename;
    
    if (LinkOpen(&filename))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_LINK, _("Open") + " " + filename);
    }
  }

  if (m_File.GetFileName().FileExists() && sel.empty())
  {
    if (wxExVCS::DirExists(m_File.GetFileName()))
    {
      menu.AppendSeparator();
      menu.AppendVCS(m_File.GetFileName());
    }
  }

  if (sel.empty() && 
      (m_Lexer.GetScintillaLexer() == "hypertext" ||
       m_Lexer.GetScintillaLexer() == "xml"))
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_OPEN_BROWSER, _("&Open In Browser"));
  }

  if (!m_vi.GetIsActive() && GetTextLength() > 0)
  {
    menu.AppendSeparator();
    menu.Append(wxID_FIND);

    if (!GetReadOnly())
    {
      menu.Append(wxID_REPLACE);
    }
  }

  menu.AppendSeparator();
  menu.AppendEdit();

  if (!GetReadOnly())
  {
    if (!sel.empty())
    {
      wxExMenu* menuSelection = new wxExMenu(menu);
      menuSelection->Append(ID_EDIT_UPPERCASE, _("&Uppercase\tF11"));
      menuSelection->Append(ID_EDIT_LOWERCASE, _("&Lowercase\tF12"));

      if (wxExGetNumberOfLines(sel) > 1)
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
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.AppendSeparator();
    if (CanUndo()) menu.Append(wxID_UNDO);
    if (CanRedo()) menu.Append(wxID_REDO);
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
     sel.empty() && 
     GetProperty("fold") == "1" &&
     m_Lexer.IsOk() &&
    !m_Lexer.GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_TOGGLE_FOLD, _("&Toggle Fold\tCtrl+T"));
    menu.Append(ID_EDIT_FOLD_ALL, _("&Fold All Lines\tF9"));
    menu.Append(ID_EDIT_UNFOLD_ALL, _("&Unfold All Lines\tF10"));
  }
}

bool wxExSTC::CanCut() const
{
  return wxStyledTextCtrl::CanCut() && !GetReadOnly() && !HexMode();
}

bool wxExSTC::CanPaste() const
{
  return wxStyledTextCtrl::CanPaste() && !GetReadOnly() && !HexMode();
}

void wxExSTC::CheckAutoComp(const wxUniChar& c)
{
  if (wxExIsCodewordSeparator(GetCharAt(GetCurrentPos() - 1)))
  {
    m_AutoComplete = c;
  }
  else
  {
    m_AutoComplete += c;

    if (m_AutoComplete.length() >= 3) // Only autocompletion for large words
    {
      if (!AutoCompActive())
      {
        AutoCompSetIgnoreCase(true);
        AutoCompSetAutoHide(false);
      }

      if (m_Lexer.KeywordStartsWith(m_AutoComplete))
        AutoCompShow(
          m_AutoComplete.length() - 1,
          m_Lexer.GetKeywordsString());
      else
        AutoCompCancel();
    }
  }
}

void wxExSTC::CheckBrace()
{
  if (HexMode())
  {
    if (!CheckBraceHex(GetCurrentPos()))
    {
      if (PositionFromLine(GetCurrentLine()) != GetCurrentPos())
      {
        CheckBraceHex(GetCurrentPos() - 1);
      }
    }
  }
  else if (!CheckBrace(GetCurrentPos()))
  {
    CheckBrace(GetCurrentPos() - 1);
  }
}

bool wxExSTC::CheckBrace(int pos)
{
  const int brace_match = BraceMatch(pos);

  if (brace_match != wxSTC_INVALID_POSITION)
  {
    BraceHighlight(pos, brace_match);
    return true;
  }
  else
  {
    BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
    return false;
  }
}

bool wxExSTC::CheckBraceHex(int pos)
{
  const int brace_match = wxExHexModeLine(this).OtherField();
  
  if (brace_match != wxSTC_INVALID_POSITION)
  {
    BraceHighlight(pos, PositionFromLine(LineFromPosition(pos)) + brace_match);
    return true;
  }
  else
  {
    BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
    return false;
  }
}

void wxExSTC::ClearDocument(bool set_savepoint)
{
  SetReadOnly(false);
  
  ClearAll();
  
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(wxEmptyString, "PaneInfo");
#endif

  if (set_savepoint)
  {
    EmptyUndoBuffer();
    SetSavePoint();
  }
  
  m_HexBuffer.clear();
}

// This is a static method, cannot use normal members here.
int wxExSTC::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  long flags,
  wxWindowID id)
{
  std::vector<wxExConfigItem> items;

  // General page.
  std::set<wxString> bchoices;
  bchoices.insert(_("End of line"));
  bchoices.insert(_("Line numbers"));
  bchoices.insert(_("Use tabs"));
  bchoices.insert(_("Caret line"));
  bchoices.insert(_("Scroll bars"));
  bchoices.insert(_("vi mode"));
  // use 2 cols here, but 1 for others on this page
  items.push_back(wxExConfigItem(bchoices, _("General") + ":2")); 

  std::map<long, const wxString> choices;
  choices.insert(std::make_pair(wxSTC_WS_INVISIBLE, _("Invisible")));
  choices.insert(std::make_pair(wxSTC_WS_VISIBLEAFTERINDENT, 
    _("Invisible after ident")));
  choices.insert(std::make_pair(wxSTC_WS_VISIBLEALWAYS, _("Visible always")));
  items.push_back(wxExConfigItem(
    _("Whitespace"), choices, true, _("General"), 1));

  // Next code does not have any effect (2.9.5, on MSW and GTK)
/*  
  std::map<long, const wxString> smode;
  smode.insert(std::make_pair(wxSTC_SEL_STREAM, _("Stream")));
  smode.insert(std::make_pair(wxSTC_SEL_RECTANGLE, _("Rectangular")));
  smode.insert(std::make_pair(wxSTC_SEL_THIN, _("Thin")));
  smode.insert(std::make_pair(wxSTC_SEL_LINES, _("Lines")));
  items.push_back(wxExConfigItem(
    _("Selection mode"), 
    smode, 
    true, 
    _("General"),
    1));
*/
  
  std::map<long, const wxString> wchoices;
  wchoices.insert(std::make_pair(wxSTC_WRAP_NONE, _("None")));
  wchoices.insert(std::make_pair(wxSTC_WRAP_WORD, _("Word")));
  wchoices.insert(std::make_pair(wxSTC_WRAP_CHAR, _("Char")));
  items.push_back(wxExConfigItem(
    _("Wrap line"), wchoices, true, _("General"), 1));

  std::map<long, const wxString> vchoices;
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_NONE, _("None")));
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_END, _("End")));
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_START, _("Start")));
  items.push_back(wxExConfigItem(
    _("Wrap visual flags"), vchoices, true, _("General"), 1));
    
  if (wxExLexers::Get()->GetCount() > 0)
  {
    items.push_back(wxExConfigItem(_("Default font"), CONFIG_FONTPICKERCTRL));
  }

  // Edge page.
  items.push_back(wxExConfigItem(_("Edge column"), 0, 500, _("Edge")));
  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(wxSTC_EDGE_NONE, _("None")));
  echoices.insert(std::make_pair(wxSTC_EDGE_LINE, _("Line")));
  echoices.insert(std::make_pair(wxSTC_EDGE_BACKGROUND, _("Background")));
  items.push_back(wxExConfigItem(
    _("Edge line"), echoices, true, _("Edge"), 1));

  // Margin page.
  items.push_back(wxExConfigItem(
    _("Tab width"), 1, (int)wxConfigBase::Get()->ReadLong(_("Edge column"), 80), 
    _("Margin")));
  items.push_back(wxExConfigItem(
    _("Indent"), 0, (int)wxConfigBase::Get()->ReadLong(_("Edge column"), 80), _("Margin")));
  items.push_back(wxExConfigItem(
    _("Divider"), 0, 40, _("Margin")));
  if (wxExLexers::Get()->GetCount() > 0)
  {
    items.push_back(wxExConfigItem(
      _("Folding"), 0, 40, _("Margin")));
  }
  items.push_back(wxExConfigItem(
    _("Line number"), 0, 100, _("Margin")));

  if (wxExLexers::Get()->GetCount() > 0)
  {
    // Folding page.
    items.push_back(wxExConfigItem(_("Indentation guide"), CONFIG_CHECKBOX,
      _("Folding")));
    items.push_back(wxExConfigItem(_("Auto fold"), 0, INT_MAX, _("Folding")));

    std::map<long, const wxString> fchoices;
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED,
      _("Line before expanded")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED,
      _("Line before contracted")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEAFTER_EXPANDED,
      _("Line after expanded")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED,
      _("Line after contracted")));
    // next is experimental, wait for scintilla
    //fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")));
    items.push_back(wxExConfigItem(_("Fold flags"), fchoices, false,
      _("Folding")));
  }
  
  // Printer page
  std::map<long, const wxString> pchoices;
  pchoices.insert(std::make_pair(wxSTC_PRINT_NORMAL, _("Normal")));
  pchoices.insert(std::make_pair(wxSTC_PRINT_INVERTLIGHT, _("Invert on white")));
  pchoices.insert(std::make_pair(wxSTC_PRINT_BLACKONWHITE, _("Black on white")));
  pchoices.insert(std::make_pair(wxSTC_PRINT_COLOURONWHITE, _("Colour on white")));
  pchoices.insert(std::make_pair(wxSTC_PRINT_COLOURONWHITEDEFAULTBG, _("Colour on white normal")));
  items.push_back(wxExConfigItem(
    _("Print flags"), pchoices, true, _("Printer"), 1));

  if (!(flags & STC_CONFIG_SIMPLE) && wxExLexers::Get()->GetCount() > 0)
  {
    // Directory page.
    items.push_back(wxExConfigItem(
      _("Include directory"), CONFIG_LISTVIEW_FOLDER, _("Directory"), false, wxID_ANY, 25, false)); // no label
  }

  int buttons = wxOK | wxCANCEL;

  if (flags & STC_CONFIG_WITH_APPLY)
  {
    buttons |= wxAPPLY;
  }
  
  const int style = wxExConfigDialog::CONFIG_NOTEBOOK;

  if (!(flags & STC_CONFIG_MODELESS))
  {
    return wxExConfigDialog(
      parent, items, title, 0, 1, buttons, id, style).ShowModal();
  }
  else
  {
    if (m_ConfigDialog == NULL)
    {
      m_ConfigDialog = new wxExConfigDialog(
        parent, items, title, 0, 1, buttons, id, style);
    }

    return m_ConfigDialog->Show();
  }
}

void wxExSTC::ConfigGet(bool init)
{
  wxConfigBase* cfg = wxConfigBase::Get();
  
  if (!cfg->Exists(_("Caret line")))
  {
    cfg->SetRecordDefaults(true);
  }
  
  const wxFont font(cfg->ReadObject(_("Default font"), wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));

  if (m_DefaultFont != font)
  {
    m_DefaultFont = font;
    
    StyleResetDefault();
    
    // Doing this once is enough, not yet possible.
    wxExLexers::Get()->LoadDocument();
    
    SetLexer(m_Lexer);
  }

  if (m_File.GetFileName().GetExt().CmpNoCase("log") == 0)
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }
  else
  {
    SetEdgeColumn(cfg->ReadLong(_("Edge column"), 80));
    SetEdgeMode(cfg->ReadLong(_("Edge line"), wxSTC_EDGE_NONE));
  }
  
  SetUseHorizontalScrollBar(cfg->ReadBool(_("Scroll bars"), true));
  SetUseVerticalScrollBar(cfg->ReadBool(_("Scroll bars"), true));
  SetPrintColourMode(cfg->ReadLong(_("Print flags"), wxSTC_PRINT_BLACKONWHITE));
  SetCaretLineVisible(cfg->ReadBool(_("Caret line"), false));
    
  SetFoldFlags(cfg->ReadLong( _("Fold flags"),
    wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
  const long def_tab_width = 2;
  SetIndent(cfg->ReadLong(_("Indent"), def_tab_width));
  SetIndentationGuides( cfg->ReadBool(_("Indentation guide"), false));
  SetMarginWidth(m_MarginDividerNumber,  cfg->ReadLong(_("Divider"), 16));

  if (init)
  {
    Fold();
  }
    
  ShowLineNumbers(cfg->ReadBool(_("Line numbers"), false));

//  SetSelectionMode(cfg->ReadLong(_("Selection mode"), wxSTC_SEL_STREAM));
    
  SetTabWidth(cfg->ReadLong(_("Tab width"), def_tab_width));
  SetUseTabs(cfg->ReadBool(_("Use tabs"), false));
  SetViewEOL(cfg->ReadBool(_("End of line"), false));
  SetViewWhiteSpace(cfg->ReadLong(_("Whitespace"), wxSTC_WS_INVISIBLE));
  SetWrapMode(cfg->ReadLong(_("Wrap line"), wxSTC_WRAP_NONE));
  SetWrapVisualFlags(cfg->ReadLong(_("Wrap visual flags"),  wxSTC_WRAPVISUALFLAG_END));

  // Here the default vi mode is set, and used if the application
  // is run for the first time.
  m_vi.Use(cfg->ReadBool(_("vi mode"), false));

  m_Link.SetFromConfig();

  if (cfg->IsRecordingDefaults())
  {
    // Set defaults only.
    cfg->ReadLong(_("Auto fold"), 1500);
    cfg->ReadLong(_("Folding"), 16);
    cfg->ReadBool(_("Scroll bars"), true);

    cfg->SetRecordDefaults(false);
  }
}

void wxExSTC::ControlCharDialog(const wxString& caption)
{
  if (GetSelectedText().length() > 2)
  {
    // Do nothing
    return;
  }
  
  if (HexMode())
  {
    wxExHexModeLine ml(this, GetSelectionStart());
    
    if (
      ml.IsAsciiField() &&
      GetSelectedText().length() == 1)
    {
      const wxUniChar value = GetSelectedText().GetChar(0);

      long new_value;
      if ((new_value = wxExGetHexNumberFromUser(_("Input") + " 00 - FF",
        wxEmptyString,
        caption,
        value,
        0,
        255,
        this)) < 0)
      {
        return;
      }
      
      ml.Replace(new_value);
    }
    else if (
      ml.IsHexField() &&
      GetSelectedText().length() == 2)
    {
      long value;
      
      if (!GetSelectedText().ToLong(&value, 16))
      {
        return;
      }

      long new_value;
      if ((new_value = wxExGetHexNumberFromUser(_("Input") + " 00 - FF",
        wxEmptyString,
        caption,
        value,
        0,
        255,
        this)) < 0)
      {
        return;
      }
      
      ml.ReplaceHex(new_value);
    }
    
    return;
  }

  if (GetReadOnly())
  {
    if (GetSelectedText().length() == 1)
    {
      const wxUniChar value = GetSelectedText().GetChar(0);
      wxMessageBox(
        wxString::Format("hex: %x dec: %d", value, value), 
        _("Control Character"));
    }

    return;
  }

  static int value = ' '; // don't use 0 as default as NULL is not handled

  if (GetSelectedText().length() == 1)
  {
    value = GetSelectedText().GetChar(0);
  }

  int new_value;
  
  if ((new_value = (int)wxGetNumberFromUser(_("Input") + " 0 - 255:",
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

    AddTextRaw(buffer, 1);
    
    ProcessChar(new_value);
  }
  
  value = new_value;
}

void wxExSTC::Copy()
{
  if (CanCopy()) 
  {
    wxStyledTextCtrl::Copy();
      
    if (m_vi.GetIsActive())
    {
      m_vi.SetRegisterYank(GetSelectedText());
    }
  }
}

void wxExSTC::Cut()
{
  if (CanCut()) 
  {
    if (m_vi.GetIsActive())
    {
      m_vi.SetRegistersDelete(GetSelectedText());
    }
  
    wxStyledTextCtrl::Cut();
  }
}
  
void wxExSTC::EOLModeUpdate(int eol_mode)
{
  if (GetReadOnly())
  {
    wxLogStatus(_("Document is readonly"));
    return;
  }
  else
  {
    if (HexMode())
    {
      wxLogStatus(_("Not allowed in hex mode"));
      return;
    }
    else
    {
      ConvertEOLs(eol_mode);
    }
  }
  
  SetEOLMode(eol_mode);
  
#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
}

bool wxExSTC::FileReadOnlyAttributeChanged()
{
  SetReadOnly(!m_File.GetFileName().IsFileWritable()); // does not return anything
  wxLogStatus(_("Readonly attribute changed"));

  return true;
}

void wxExSTC::FileTypeMenu()
{
  wxMenu* menu = new wxMenu();

  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  menu->AppendRadioItem(ID_EDIT_EOL_DOS, "&DOS");
  menu->AppendRadioItem(ID_EDIT_EOL_MAC, "&MAC");
  menu->AppendRadioItem(ID_EDIT_EOL_UNIX, "&UNIX");
  menu->AppendSeparator();
  wxMenuItem* hex = menu->AppendCheckItem(ID_EDIT_HEX, "&HEX");
  
  menu->FindItemByPosition(GetEOLMode())->Check();
  
  if (HexMode())
  {
    hex->Check();
  }

  PopupMenu(menu);
  
  delete menu;
}

bool wxExSTC::FindNext(bool find_next)
{
  return FindNext(
    wxExFindReplaceData::Get()->GetFindString(),
    wxExFindReplaceData::Get()->STCFlags(),
    find_next);
}

bool wxExSTC::FindNext(
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

  if (SearchInTarget(text) == -1)
  {
    wxExFrame::StatusText(
      wxExGetFindResult(text, find_next, recursive), wxEmptyString);
    
    bool found = false;
    
    if (!recursive)
    {
      recursive = true;
      found = FindNext(text, search_flags, find_next);
      recursive = false;
    }
    
    return found;
  }
  else
  {
    if (!recursive)
    {
      wxLogStatus(wxEmptyString);
    }
    
    recursive = false;

    switch (m_vi.GetMode())
    {
      case wxExVi::MODE_NORMAL:
        SetSelection(GetTargetStart(), GetTargetEnd());
        EnsureVisible(LineFromPosition(GetTargetStart()));
      break;
      case wxExVi::MODE_VISUAL:
        SetSelection(GetSelectionStart(), GetTargetEnd());
        EnsureVisible(LineFromPosition(GetTargetEnd()));
      break;
      case wxExVi::MODE_VISUAL_LINE:
        {
        int begin = PositionFromLine(LineFromPosition(GetSelectionStart()));
        int end = PositionFromLine(LineFromPosition(GetTargetEnd()) + 1);
        SetSelection(begin, end);
        EnsureVisible(LineFromPosition(GetTargetEnd()));
        }
      break;
    }
      
    EnsureCaretVisible();

    return true;
  }
}

void wxExSTC::Fold(bool foldall)
{
  if (
     GetProperty("fold") == "1" &&
     m_Lexer.IsOk() &&
    !m_Lexer.GetScintillaLexer().empty())
  {
    SetMarginWidth(m_MarginFoldingNumber, 
      wxConfigBase::Get()->ReadLong(_("Folding"), 16));
    SetFoldFlags(
      wxConfigBase::Get()->ReadLong(_("Fold flags"),
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | 
        wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
        
    if (
      foldall || 
      GetLineCount() > wxConfigBase::Get()->ReadLong(_("Auto fold"), 1500))
    {
      FoldAll();
    }
  }
  else
  {
    SetMarginWidth(m_MarginFoldingNumber, 0);
  }
}
  
void wxExSTC::FoldAll()
{
  if (GetProperty("fold") != "1") return;

  const int current_line = GetCurrentLine();
  const bool xml = (m_Lexer.GetDisplayLexer() == "xml");

  int line = 0;
  while (line < GetLineCount())
  {
    const int level = GetFoldLevel(line);
    const int last_child_line = GetLastChild(line, level);
    
    if (xml && (
        level == wxSTC_FOLDLEVELBASE + wxSTC_FOLDLEVELHEADERFLAG))
    {
      line++;
    }
    else if (last_child_line > line + 1)
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

const wxString wxExSTC::GetEOL() const
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

// Cannot be const because of GetSelectedText (not const in 2.9.4).
const wxString wxExSTC::GetFindString()
{
  const wxString selection = GetSelectedText();

  if (!selection.empty() && wxExGetNumberOfLines(selection) == 1)
  {
    bool alnum = true;
    
    // If regexp is true, then only use selected text if text does not
    // contain special regexp characters.
    if (wxExFindReplaceData::Get()->STCFlags() & wxSTC_FIND_REGEXP)
    {
      for (size_t i = 0; i < selection.size() && alnum; i++)
      {
        if (
          !isalnum(selection[i]) && 
           selection[i] != ' ' && 
           selection[i] != '_')
        {
          alnum = false;
        }
      }
    }

    if (alnum)
    {  
      wxExFindReplaceData::Get()->SetFindString(selection);
    }
  }

  return wxExFindReplaceData::Get()->GetFindString();
}

const wxString wxExSTC::GetWordAtPos(int pos) const
{
  const int word_start = 
    const_cast< wxExSTC * >( this )->WordStartPosition(pos, true);
  const int word_end = 
    const_cast< wxExSTC * >( this )->WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const wxString word = 
      const_cast< wxExSTC * >( this )->GetTextRange(word_start, word_start + 1);

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
      const_cast< wxExSTC * >( this )->GetTextRange(word_start, word_end);

    return word;
  }
}

bool wxExSTC::GotoDialog()
{
  if (HexMode())
  {
    long val;
    if ((val = wxGetNumberFromUser(
      _("Input") + wxString::Format(" 0 - %d:", m_HexBuffer.length() - 1),
      wxEmptyString,
      _("Enter Byte Offset"),
      m_Goto, // initial value
      0,
      m_HexBuffer.length() - 1,
      this)) < 0)
    {
      return false;
    }

    m_Goto = val;
    
    wxExHexModeLine(this, val, false).Goto();
  }
  else
  {
    if (m_Goto > GetLineCount())
    {
      m_Goto = GetLineCount();
    }
    else if (m_Goto < 1)
    {
      m_Goto = 1;
    }

    long val;
    if ((val = wxGetNumberFromUser(
      _("Input") + wxString::Format(" 1 - %d:", GetLineCount()),
      wxEmptyString,
      _("Enter Line Number"),
      m_Goto, // initial value
      1,
      GetLineCount(),
      this)) < 0)
    {
      return false;
    }

    GotoLineAndSelect(val);
  }

  return true;
}

void wxExSTC::GotoLineAndSelect(
  int line_number, 
  const wxString& text,
  int col_number)
{
  // line_number and m_Goto start with 1 and is allowed to be 
  // equal to number of lines.
  // Internally GotoLine starts with 0, therefore 
  // line_number - 1 is used afterwards.
  if (line_number > GetLineCount())
  {
    line_number = GetLineCount();
  }
  else if (line_number <= 0) 
  {
    line_number = 1;
  }

  GotoLine(line_number - 1);
  EnsureVisible(line_number - 1);
  EnsureCaretVisible();

  m_Goto = line_number;

  if (!text.empty())
  {
    SetSearchFlags(wxExFindReplaceData::Get()->STCFlags());
    const int start_pos = PositionFromLine(line_number - 1);
    const int end_pos = GetLineEndPosition(line_number - 1);

    SetTargetStart(start_pos);
    SetTargetEnd(end_pos);

    if (SearchInTarget(text) != -1)
    {
      SetSelection(GetTargetStart(), GetTargetEnd());
    }
  }
  else if (col_number > 0)
  {
    const int max = GetLineEndPosition(line_number - 1);
    const int asked = GetCurrentPos() + col_number - 1;
    
    SetCurrentPos(asked < max ? asked: max);
    
    // Reset selection, seems necessary.
    SelectNone();
  }
}

void wxExSTC::GuessType()
{
  // Get a small sample from this document to detect the file mode.
  const int length = (!HexMode() ? GetTextLength(): m_HexBuffer.size());
  const int sample_size = (length > 255 ? 255: length);
  
  const wxString text = (!HexMode() ? GetTextRange(0, sample_size): 
    m_HexBuffer.Mid(0, sample_size));
  
  const wxRegEx ex(".*vi: *set .*");
    
  if (ex.Matches(text))
  {
    m_vi.Command(":" + text.AfterFirst(':').Trim(false));
  }

  if      (text.Contains("\r\n")) SetEOLMode(wxSTC_EOL_CRLF);
  else if (text.Contains("\n"))   SetEOLMode(wxSTC_EOL_LF);
  else if (text.Contains("\r"))   SetEOLMode(wxSTC_EOL_CR);
  else return; // do nothing

#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
}

void wxExSTC::HexDecCalltip(int pos)
{
  if (CallTipActive())
  {
    CallTipCancel();
  }
  
  if (HexMode())
  {
    CallTipShow(pos, wxExHexModeLine(this).GetInfo());
    return;
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

void wxExSTC::Initialize(bool file_exists)
{
  SetSearchFlags(wxSTC_FIND_REGEXP);
  Sync();
  
  m_HexMode = false;
  
  if (m_Flags & STC_WIN_HEX)
  {
    SetHexMode(true);
  }

  m_AddingChars = false;
  m_AllowChangeIndicator = !(m_Flags & STC_WIN_NO_INDICATOR);
  
  m_HexBuffer.clear(); // always, not only in hex mode

  m_FoldLevel = 0;
  m_SavedPos = -1;
  m_SavedSelectionStart = -1;
  m_SavedSelectionEnd = -1;
  
  if (wxExLexers::Get()->GetCount() > 0)
  {
    m_DefaultFont = wxConfigBase::Get()->ReadObject(
      _("Default font"), wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT));
  }
  
#ifdef __WXMSW__
  SetEOLMode(wxSTC_EOL_CRLF);
#elif __WXGTK__
  SetEOLMode(wxSTC_EOL_LF);
#else
  SetEOLMode(wxSTC_EOL_CR);
#endif

  SetBackSpaceUnIndents(true);
  SetMouseDwellTime(1000);
  SetMarginType(m_MarginLineNumber, wxSTC_MARGIN_NUMBER);
  SetMarginType(m_MarginDividerNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_MarginFoldingNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginMask(m_MarginFoldingNumber, wxSTC_MASK_FOLDERS);
  SetMarginSensitive(m_MarginFoldingNumber, true);
  
  if (m_Zoom == -1)
  {
    m_Zoom = GetZoom();
  }
  else
  {
    SetZoom(m_Zoom);
  }

  UsePopUp(false); // we have our own

  const int accels = 20; // take max number of entries
  wxAcceleratorEntry entries[accels];

  int i = 0;

  entries[i++].Set(wxACCEL_CTRL, (int)'Z', wxID_UNDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'Y', wxID_REDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'D', ID_EDIT_HEX_DEC_CALLTIP);
  entries[i++].Set(wxACCEL_CTRL, (int)'K', ID_EDIT_CONTROL_CHAR);
  entries[i++].Set(wxACCEL_CTRL, '=', ID_EDIT_ZOOM_IN);
  entries[i++].Set(wxACCEL_CTRL, '-', ID_EDIT_ZOOM_OUT);
  entries[i++].Set(wxACCEL_CTRL, '9', ID_EDIT_MARKER_NEXT);
  entries[i++].Set(wxACCEL_CTRL, '0', ID_EDIT_MARKER_PREVIOUS);
  entries[i++].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F7, wxID_SORT_ASCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F8, wxID_SORT_DESCENDING);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F9, ID_EDIT_FOLD_ALL);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F10, ID_EDIT_UNFOLD_ALL);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F11, ID_EDIT_UPPERCASE);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F12, ID_EDIT_LOWERCASE);
  entries[i++].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[i++].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);

  wxAcceleratorTable accel(i, entries);
  SetAcceleratorTable(accel);

  // Prevent doing this double, see wxExLexer::Apply.
  if (!file_exists)
  {
    wxExLexers::Get()->ApplyGlobalStyles(this);
  }
  
  ConfigGet(true);
}

bool wxExSTC::LinkOpen(wxString* filename)
{
  const wxString sel = GetSelectedText();
  const wxString text = (!sel.empty() ? sel: GetCurLine());
  int line_no = 0;
  int col_no = 0;
  const wxString path = m_Link.GetPath(text, line_no, col_no);
  
  if (!path.empty())
  {
    if (filename == NULL)
    {
      wxExFrame* frame = dynamic_cast<wxExFrame*>(wxTheApp->GetTopWindow());
      
      if (frame != NULL)
      {
        return frame->OpenFile(
          path,
          line_no, 
          wxEmptyString,
          col_no,
          GetFlags() | STC_WIN_FROM_OTHER);          
      }
      else
      {
        return Open(
          path, 
          line_no, 
          wxEmptyString, 
          col_no,
          GetFlags() | STC_WIN_FROM_OTHER);
      }
    }
    else
    {
      *filename = wxFileName(path).GetFullName();
    }
  }
  
  return !path.empty();
}

bool wxExSTC::MarkerDeleteAllChange()
{
  if (!wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return false;
  }
  
  MarkerDeleteAll(m_MarkerChange.GetNo());
  
  return true;
}
  
void wxExSTC::MarkModified(const wxStyledTextEvent& event)
{
  if (!wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return;
  }
  
  UseModificationMarkers(false);
  
  const int line = LineFromPosition(event.GetPosition());
  
  if (event.GetModificationType() & wxSTC_PERFORMED_UNDO)
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
  
void wxExSTC::MarkerNext(bool next)
{
  int line = (next ? 
    wxStyledTextCtrl::MarkerNext(GetCurrentLine() + 1, 0xFFFF):
    wxStyledTextCtrl::MarkerPrevious(GetCurrentLine() - 1, 0xFFFF));
    
  if (line == -1)
  {
    line = (next ?
      wxStyledTextCtrl::MarkerNext(0, 0xFFFF):
      wxStyledTextCtrl::MarkerPrevious(GetLineCount() - 1, 0xFFFF));
  }
    
  if (line != -1)
  {
    GotoLine(line);
  }
  else
  {
    wxLogStatus(_("No markers present"));
  }
}

void wxExSTC::OnChar(wxKeyEvent& event)
{
  if (!m_vi.GetIsActive())
  {
    if (wxIsalnum(event.GetUnicodeKey()))
    {
      m_AddingChars = true;
    }
  }
  else if (m_vi.GetMode() == wxExVi::MODE_INSERT)
  {
    if (wxIsalnum(event.GetUnicodeKey()))
    {
      m_AddingChars = true;
    }
  
    CheckAutoComp(event.GetUnicodeKey());
  }
  else
  {
    m_AddingChars = false;
  }

  if (m_vi.OnChar(event))
  {
    if (
      GetReadOnly() && 
      wxIsalnum(event.GetUnicodeKey()))
    {
      wxLogStatus(_("Document is readonly"));
      return;
    }

    if (HexMode())
    {
      if (GetOvertype())
      {
        if (wxExHexModeLine(this).Replace(event.GetUnicodeKey()))
        {
          CharRight();
        }
      }
      
      return;
    }
    
    if (!m_vi.GetIsActive())
    {
      CheckAutoComp(event.GetUnicodeKey());
    }
  
    event.Skip();
  }
}

void wxExSTC::OnCommand(wxCommandEvent& command)
{
  switch (command.GetId())
  {
    case wxID_COPY: Copy(); break;
    case wxID_CUT: Cut(); break;
    case wxID_DELETE: if (!GetReadOnly() && !HexMode()) Clear(); break;
    case wxID_JUMP_TO: GotoDialog(); break;
    case wxID_PASTE: Paste(); break;
    case wxID_SELECTALL: SelectAll(); break;
    case wxID_UNDO: Undo(); break;
    case wxID_REDO: Redo(); break;
    case wxID_SAVE: m_File.FileSave(); break;
    case wxID_SORT_ASCENDING: SortSelectionDialog(true); break;
    case wxID_SORT_DESCENDING: SortSelectionDialog(false); break;
    
    case wxID_FIND: 
    case wxID_REPLACE: 
      GetFindString();
      command.Skip();
      break;

    case ID_EDIT_FIND_NEXT: 
    case ID_EDIT_FIND_PREVIOUS: 
      GetFindString();
      FindNext(command.GetId() == ID_EDIT_FIND_NEXT); 
      break;
      
    case ID_EDIT_CONTROL_CHAR: ControlCharDialog(); break;
    case ID_EDIT_EOL_DOS: EOLModeUpdate(wxSTC_EOL_CRLF); break;
    case ID_EDIT_EOL_UNIX: EOLModeUpdate(wxSTC_EOL_LF); break;
    case ID_EDIT_EOL_MAC: EOLModeUpdate(wxSTC_EOL_CR); break;
    case ID_EDIT_HEX: Reload(m_Flags ^ STC_WIN_HEX); break;

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

    case ID_EDIT_HEX_DEC_CALLTIP:
      HexDecCalltip(GetCurrentPos());
    break;

    case ID_EDIT_LOWERCASE: LowerCase(); break;
    case ID_EDIT_UPPERCASE: UpperCase(); break;
    
    case ID_EDIT_MARKER_NEXT: MarkerNext(true); break;
    case ID_EDIT_MARKER_PREVIOUS: MarkerNext(false); break;
    
    case ID_EDIT_OPEN_BROWSER:
      wxLaunchDefaultBrowser(m_File.GetFileName().GetFullPath());
      break;

    case ID_EDIT_OPEN_LINK: LinkOpen(); break;

    case ID_EDIT_READ: m_File.Read(command.GetString()); break;
      
    case ID_EDIT_SHOW_PROPERTIES: ShowProperties(); break;
      
    case ID_EDIT_ZOOM_IN: SetZoom(++m_Zoom); break;
    case ID_EDIT_ZOOM_OUT: SetZoom(--m_Zoom);  break;

    default: wxFAIL; break;
  }
}

void wxExSTC::OnFindDialog(wxFindDialogEvent& event)
{
  wxExFindReplaceData* frd = wxExFindReplaceData::Get();

  if (
    event.GetEventType() == wxEVT_COMMAND_FIND ||
    event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
  {
    FindNext(frd->SearchDown());
  }
  else if (event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE)
  {
    ReplaceNext(frd->SearchDown());
  }
  else if (event.GetEventType() == wxEVT_COMMAND_FIND_REPLACE_ALL)
  {
    ReplaceAll(
      frd->GetFindString(), 
      frd->GetReplaceString());
  }
  else
  {
    wxFAIL;
  }
}

void wxExSTC::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    m_File.CheckSync() &&
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !(m_Flags & STC_WIN_READ_ONLY) &&
      m_File.GetFileName().GetStat().IsReadOnly() != GetReadOnly())
  {
    FileReadOnlyAttributeChanged();
  }
}

void wxExSTC::OnKeyDown(wxKeyEvent& event)
{
  if (event.GetModifiers() == wxMOD_ALT)
  {
    return;
  }
  
  if (HexMode())
  {  
    if (
      event.GetKeyCode() == WXK_LEFT ||
      event.GetKeyCode() == WXK_RIGHT)
    {
      wxExHexModeLine(this).SetPos(event);
    }
  }

  if (m_vi.OnKeyDown(event))
  {
    event.Skip();
  }
}

void wxExSTC::OnKeyUp(wxKeyEvent& event)
{
  event.Skip();
    
  CheckBrace();
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;
}

void wxExSTC::OnMouse(wxMouseEvent& event)
{
  if (event.LeftUp())
  {
    PropertiesMessage();
    event.Skip();
    CheckBrace();
      
    m_AddingChars = false;
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;
  }
  else if (event.RightUp())
  {
    if (m_MenuFlags == 0)
    {
      event.Skip();
    }
    else
    {
      int style = 0; // otherwise CAN_PASTE already on
      
      if ( GetReadOnly() || HexMode()) style |= wxExMenu::MENU_IS_READ_ONLY;
      if (!GetSelectedText().empty())  style |= wxExMenu::MENU_IS_SELECTED;
      if ( GetTextLength() == 0)       style |= wxExMenu::MENU_IS_EMPTY;
      if ( CanPaste())                 style |= wxExMenu::MENU_CAN_PASTE;

      wxExMenu menu(style);
      
      BuildPopupMenu(menu);
      
      if (menu.GetMenuItemCount() > 0)
      {
        // If last item is a separator, delete it.
        wxMenuItem* item = menu.FindItemByPosition(menu.GetMenuItemCount() - 1);
      
        if (item->IsSeparator())
        {
          menu.Delete(item->GetId());
        }
      
        PopupMenu(&menu);
      }
    }
  }
  else
  {
    wxFAIL;
  }
}

void wxExSTC::OnStyledText(wxStyledTextEvent& event)
{
  if (event.GetEventType() == wxEVT_STC_MODIFIED)
  {
    MarkModified(event); 
      
    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_STC_CHARADDED)
  {
    event.Skip();
    
    AutoIndentation(event.GetKey());
  }
  else if (event.GetEventType() == wxEVT_STC_START_DRAG)
  {
#if wxUSE_DRAG_AND_DROP
    if (HexMode() || GetReadOnly())
    {
      event.SetDragAllowMove(false);
    }
#endif    
    
    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_STC_DO_DROP)
  {
#if wxUSE_DRAG_AND_DROP
    if (HexMode() || GetReadOnly())
    {
      event.SetDragResult(wxDragNone);
    }
#endif    
    
    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_STC_DWELLEND)
  {
    if (CallTipActive())
    {
      CallTipCancel();
    }
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
  else if (event.GetEventType() == wxEVT_STC_AUTOCOMP_SELECTION)
  {
    if (m_vi.GetIsActive())
    {
      m_vi.Command(event.GetText().Mid(m_AutoComplete.size()));
    }
  }
  else
  {
    wxFAIL;
  }
}

bool wxExSTC::Open(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags)
{
  if (m_File.GetFileName() == filename && line_number > 0)
  {
    GotoLineAndSelect(line_number, match, col_number);
    PropertiesMessage();
    return true;
  }

  m_Flags = flags;

  bool success;

  if (m_File.FileLoad(filename))
  {
    SetName(filename.GetFullPath());

    if (line_number > 0)
    {
      GotoLineAndSelect(line_number, match, col_number);
    }
    else
    {
      if (line_number == -1)
      {
        if (!match.empty())
        {
           FindNext(match, 0, false);
        }
        else
        {
          DocumentEnd();
        }
      }
      else if (!match.empty())
      {
        FindNext(match);
      }
    }

    PropertiesMessage();
    
    success = true;
  }
  else
  {
    wxExFrame::StatusText(wxEmptyString, "PaneInfo");
    success = false;
  }
  
  return success;
}

void wxExSTC::Paste()
{
  if (CanPaste())
  {
    wxStyledTextCtrl::Paste();
  }
}

bool wxExSTC::PositionRestore()
{
  if (m_SavedSelectionStart != -1 && m_SavedSelectionEnd != -1)
  {
    SetSelection(m_SavedSelectionStart, m_SavedSelectionEnd);
  }
  else if (m_SavedPos != -1)
  {
    SetSelection(m_SavedPos, m_SavedPos);
  }
  else
  {
    return false;
  }
  
  SetCurrentPos(m_SavedPos);
  
  EnsureCaretVisible();
  
  return true;
}
  
void wxExSTC::PositionSave()
{
  m_SavedPos = GetCurrentPos();
  m_SavedSelectionStart = GetSelectionStart();  
  m_SavedSelectionEnd = GetSelectionEnd();
}

#if wxUSE_PRINTING_ARCHITECTURE
void wxExSTC::Print(bool prompt)
{
  wxPrintData* data = wxExPrinting::Get()->GetHtmlPrinter()->GetPrintData();
  wxExPrinting::Get()->GetPrinter()->GetPrintDialogData().SetPrintData(*data);
  wxExPrinting::Get()->GetPrinter()->Print(this, new wxExPrintout(this), prompt);
}
#endif

#if wxUSE_PRINTING_ARCHITECTURE
void wxExSTC::PrintPreview()
{
  wxPrintPreview* preview = new wxPrintPreview(
    new wxExPrintout(this), 
    new wxExPrintout(this));

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

void wxExSTC::PropertiesMessage(long flags)
{
  wxExLogStatus(m_File.GetFileName(), flags);
  
#if wxUSE_STATUSBAR
  if (flags != STAT_SYNC)
  {
    wxExFrame::UpdateStatusBar(this, "PaneFileType");
    wxExFrame::UpdateStatusBar(this, "PaneLexer");
  }
  
  wxExFrame::UpdateStatusBar(this, "PaneInfo");
#endif
}

void wxExSTC::Reload(long flags)
{
  const bool modified = GetModify();
  
  UseModificationMarkers(false);
  
  if (flags & STC_WIN_HEX)
  {
    if (!HexMode())
    {
      const wxCharBuffer buffer = GetTextRaw(); // keep buffer
      SetHexMode(true, modified, buffer);
    }
  }
  else
  {
    SetHexMode(false, modified);
  }
  
  UseModificationMarkers(true);

  m_Flags = flags;
    
  if (
    (m_Flags & STC_WIN_READ_ONLY) || 
    (GetFileName().Exists() && !GetFileName().IsFileWritable()))
  {
    SetReadOnly(true);
  }
}

int wxExSTC::ReplaceAll(
  const wxString& find_text,
  const wxString& replace_text)
{
  if (HexMode())
  {
    wxLogStatus(_("Not allowed in hex mode"));
    return 0;
  }
  
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

  SetSearchFlags(wxExFindReplaceData::Get()->STCFlags());
  int nr_replacements = 0;

  BeginUndoAction();

  while (SearchInTarget(find_text) != -1)
  {
    bool skip_replace = false;

    // Check that the target is within the rectangular selection.
    // If not just continue without replacing.
    if (SelectionIsRectangle() && selection_from_end != 0)
    {
      const int line = LineFromPosition(GetTargetStart());
      const int start_pos = GetLineSelStartPosition(line);
      const int end_pos = GetLineSelEndPosition(line);
      const int length = GetTargetEnd() - GetTargetStart();

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
      wxExFindReplaceData::Get()->UseRegularExpression() ?
        ReplaceTargetRE(replace_text):
        ReplaceTarget(replace_text);

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

bool wxExSTC::ReplaceNext(bool find_next)
{
  return ReplaceNext(
    wxExFindReplaceData::Get()->GetFindString(),
    wxExFindReplaceData::Get()->GetReplaceString(),
    wxExFindReplaceData::Get()->STCFlags(),
    find_next);
}

bool wxExSTC::ReplaceNext(
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
    if (SearchInTarget(find_text) == -1) return false;
  }
  
  if (HexMode())
  {
    for (const auto& it : replace_text)
    {
      wxExHexModeLine(this, GetTargetStart()).Replace(it);
    }
  }
  else
  {
    wxExFindReplaceData::Get()->UseRegularExpression() ?
      ReplaceTargetRE(replace_text):
      ReplaceTarget(replace_text);
  }

  FindNext(find_text, search_flags, find_next);
  
  return true;
}

void wxExSTC::ResetLexer()
{
  m_Lexer.Reset(this);
  wxExFrame::StatusText(m_Lexer.GetDisplayLexer(), "PaneLexer");
  SetMarginWidth(m_MarginFoldingNumber, 0);
}
 
void wxExSTC::ResetMargins(bool divider_margin)
{
  SetMarginWidth(m_MarginFoldingNumber, 0);
  SetMarginWidth(m_MarginLineNumber, 0);

  if (divider_margin)
  {
    SetMarginWidth(m_MarginDividerNumber, 0);
  }
}

void wxExSTC::SelectNone()
{
  // The base styledtextctrl version uses scintilla, sets caret at 0.
  SetSelection(GetCurrentPos(), GetCurrentPos());
}

bool wxExSTC::SetHexMode(
  bool on, 
  bool modified,
  const wxCharBuffer& text)
{
  if (IsModified() || m_vi.GetMode() == wxExVi::MODE_INSERT)
  {
    return false;
  }
    
  if (on)
  {
    m_HexMode = true;
    
    SetControlCharSymbol('.');
    wxExLexers::Get()->ApplyHexStyles(this);
    wxExLexers::Get()->ApplyMarkers(this);
    
    m_Goto = 0;

    // Do not show an edge, eol or whitespace in hex mode.
    SetEdgeMode(wxSTC_EDGE_NONE);
    SetViewEOL(false);
    SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
    
    if (text.length() > 0)
    {
      ClearDocument(false);
      AppendTextHexMode(text);

      if (!modified)
      {
        EmptyUndoBuffer();
        SetSavePoint();
      }

      // This should be after SetSavePoint.      
      BeginUndoAction();
    }
  }
  else
  {
    SetControlCharSymbol(0);
    
    m_Goto = 1;
    
    if (!m_HexBuffer.empty())
    {
      const wxCharBuffer buffer = m_HexBuffer.ToAscii(); // keep buffer
      ClearDocument(false);
      AppendText(buffer);
      
      if (!modified)
      {
        EmptyUndoBuffer();
        SetSavePoint();
      }
    }
    
    if (m_HexMode)
    {
      EndUndoAction();
      m_HexMode = false;
    }
  }
 
  return true;
}

bool wxExSTC::SetIndicator(
  const wxExIndicator& indicator, 
  int start, 
  int end)
{
  if (!wxExLexers::Get()->IndicatorIsLoaded(indicator))
  {
    return false;
  }

  SetIndicatorCurrent(indicator.GetNo());
  IndicatorFillRange(start, end - start);
  
  return true;
}

bool wxExSTC::SetLexer(const wxExLexer& lexer, bool fold)
{
  if (!m_Lexer.Set(lexer, this))
  {
    return false;
  }

  SetLexerCommon(fold);
  
  return true;
}

bool wxExSTC::SetLexer(const wxString& lexer, bool fold)
{
  if (!m_Lexer.Set(lexer, this))
  {
    return false;
  }
  
  SetLexerCommon(fold);
  
  return true;
}

void wxExSTC::SetLexerCommon(bool fold)
{
  if (fold)
  {
    Fold();
  }
  
  if (HexMode() &&
      m_Lexer.GetScintillaLexer() == "po")
  {
    m_Link.AddBasePath();
  }
    
  wxExFrame::StatusText(m_Lexer.GetDisplayLexer(), "PaneLexer");
}

void wxExSTC::SetLexerProperty(const wxString& name, const wxString& value)
{
  m_Lexer.SetProperty(name, value);
  m_Lexer.Apply(this);
}

void wxExSTC::SetText(const wxString& value)
{
  ClearDocument();

  AddTextRaw((const char *)value.c_str(), value.length());

  DocumentStart();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

void wxExSTC::ShowLineNumbers(bool show)
{
  const int margin = wxConfigBase::Get()->ReadLong(
    _("Line number"), 
    TextWidth(wxSTC_STYLE_DEFAULT, "999999"));

  SetMarginWidth(
    m_MarginLineNumber, 
    show ? margin: 0);
}

void wxExSTC::ShowProperties()
{
  // Added check, otherwise scintilla crashes.
  const wxString propnames = (!m_Lexer.GetScintillaLexer().empty() ?
    PropertyNames(): wxString(wxEmptyString));
  
  wxString text;

  if (!propnames.empty())
  {
    text += "Current properties\n";
  }

  // Add global properties.  
  for (const auto& it1 : wxExLexers::Get()->GetProperties())
  {
    text += it1.GetName() + ": " + GetProperty(it1.GetName()) + "\n";
  }

  // Add lexer properties.  
  for (const auto& it2 : m_Lexer.GetProperties())
  {
    text += it2.GetName() + ": " + GetProperty(it2.GetName()) + "\n";
  }

  // Add available properties.
  if (!propnames.empty())
  {
    text += "\nAvailable properties\n";
    wxStringTokenizer tkz(propnames, "\n");
  
    while (tkz.HasMoreTokens())
    {
      const wxString prop = tkz.GetNextToken();
      text += prop + ": " + DescribeProperty(prop) + "\n";
    }
  }
  
  if (m_EntryDialog == NULL)
  {
    m_EntryDialog = new wxExSTCEntryDialog(
      this, 
      _("Properties"), 
      text, 
      wxEmptyString, 
      wxOK);
  }
  else
  {
    m_EntryDialog->GetSTC()->SetText(text);
  }
  
  m_EntryDialog->Show();
}

void wxExSTC::SortSelectionDialog(bool sort_ascending, const wxString& caption)
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
    for (const auto& it : mm)
    {
      text += it.second;
    }
  }
  else
  {
    for (
      auto it = mm.rbegin();
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

void wxExSTC::Sync(bool start)
{
  // do not use ?, compile error for gcc, as Bind is void, Unbind is bool
  if (start)
    Bind(wxEVT_IDLE, &wxExSTC::OnIdle, this, wxID_ANY);
  else
    Unbind(wxEVT_IDLE, &wxExSTC::OnIdle, this, wxID_ANY);
}

void wxExSTC::UseModificationMarkers(bool use)
{
  if (use)
    Bind(wxEVT_STC_MODIFIED, &wxExSTC::OnStyledText, this, wxID_ANY);
  else
    Unbind(wxEVT_STC_MODIFIED, &wxExSTC::OnStyledText, this, wxID_ANY);
}

#endif // wxUSE_GUI
