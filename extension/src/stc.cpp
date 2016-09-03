////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <vector>
#include <wx/config.h>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/numdlg.h>
#include <wx/tokenzr.h>
#include <wx/extension/stc.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/filename.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/hexmode.h>
#include <wx/extension/indicator.h>
#include <wx/extension/itemdlg.h>
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

enum
{
  INDENT_NONE,
  INDENT_WHITESPACE,
  INDENT_LEVEL,
  INDENT_ALL,
};

class STCDefaults : public wxExConfigDefaults
{
public:
  STCDefaults() 
  : wxExConfigDefaults(std::vector<std::tuple<wxString, wxExItemType, wxAny>> {
    std::make_tuple(_("Auto fold"), ITEM_TEXTCTRL_INT, 1500),
    std::make_tuple(_("Auto indent"), ITEM_TEXTCTRL_INT, (long)INDENT_ALL),
    std::make_tuple(_("Caret line"), ITEM_CHECKBOX, true),
    std::make_tuple(_("Default font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)),
    std::make_tuple(_("Divider"), ITEM_TEXTCTRL_INT, 16),
    std::make_tuple(_("Edge column"), ITEM_TEXTCTRL_INT, 80),
    std::make_tuple(_("Edge line"), ITEM_TEXTCTRL_INT, wxSTC_EDGE_NONE),
    std::make_tuple(_("Fold flags"), ITEM_TEXTCTRL_INT, wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED),
    std::make_tuple(_("Folding"), ITEM_TEXTCTRL_INT, 16),
    std::make_tuple(_("Indent"), ITEM_TEXTCTRL_INT, 2),
    std::make_tuple(_("Line number"), ITEM_TEXTCTRL_INT, 60),
    std::make_tuple(_("Print flags"), ITEM_TEXTCTRL_INT, wxSTC_PRINT_BLACKONWHITE),
    std::make_tuple(_("Scroll bars"), ITEM_CHECKBOX, true),
    std::make_tuple(_("Show mode"), ITEM_CHECKBOX, true),
    std::make_tuple(_("Tab font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)),
    std::make_tuple(_("Tab width"), ITEM_TEXTCTRL_INT, 2),
    std::make_tuple(_("Text font"), ITEM_FONTPICKERCTRL, wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)),
    std::make_tuple(_("vi mode"), ITEM_CHECKBOX, true)}) {;};
};
  
wxExItemDialog* wxExSTC::m_ConfigDialog = nullptr;
wxExSTCEntryDialog* wxExSTC::m_EntryDialog = nullptr;
int wxExSTC::m_Zoom = -1;

wxExSTC::wxExSTC(wxWindow *parent, 
  const wxString& value,
  long win_flags,
  const wxString& title,
  long menu_flags,
  const std::string& command,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size, 
  long style)
  : wxStyledTextCtrl(parent, id , pos, size, style, title)
  , m_Flags(win_flags)
  , m_MenuFlags(menu_flags)
  , m_vi(wxExVi(this))
  , m_File(this, title)
  , m_Link(wxExLink(this))
  , m_HexMode(wxExHexMode(this))
  , m_Frame(dynamic_cast<wxExFrame*>(wxTheApp->GetTopWindow()))
  , m_Lexer(this)
{
  Initialize(false);

  PropertiesMessage();

  if (!value.empty())
  {
    if (HexMode())
    {
      m_HexMode.AppendText(value.c_str());
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

  m_vi.Command(command);
}

wxExSTC::wxExSTC(wxWindow* parent,
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags,
  long menu_flags,
  const std::string& command,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxStyledTextCtrl(parent, id, pos, size, style)
  , m_File(this)
  , m_Flags(flags)
  , m_MenuFlags(menu_flags)
  , m_vi(wxExVi(this))
  , m_Link(wxExLink(this))
  , m_HexMode(wxExHexMode(this))
  , m_Frame(dynamic_cast<wxExFrame*>(wxTheApp->GetTopWindow()))
  , m_Lexer(this)
{
  Initialize(filename.GetStat().IsOk());
  
  if (filename.GetStat().IsOk())
  {
    Open(filename, line_number, match, col_number, flags, command);
  }
}

bool wxExSTC::AutoIndentation(int c)
{
  const long ai = wxConfigBase::Get()->ReadLong(_("Auto indent"), INDENT_ALL);
  
  if (ai == INDENT_NONE)
  {
    return false;
  }
  
  bool is_nl = false;
  
  switch (GetEOLMode())
  {
    case wxSTC_EOL_CR:   is_nl = (c == '\r'); break;
    case wxSTC_EOL_CRLF: is_nl = (c == '\n'); break; // so ignore first \r
    case wxSTC_EOL_LF:   is_nl = (c == '\n'); break;
  }
  
  const int currentLine = GetCurrentLine();
  
  if (!is_nl || currentLine == 0)
  {
    return false;
  }

  const int level = 
    (GetFoldLevel(currentLine) & wxSTC_FOLDLEVELNUMBERMASK) 
    - wxSTC_FOLDLEVELBASE;
  
  int indent = 0;
    
  if (level <= 0)
  {
    // the current line has yet no indents, so use previous line
    indent = GetLineIndentation(currentLine - 1);
    
    if (indent == 0)
    {
      return false;
    }
  }
  else
  {
    indent = GetIndent() * level;
  }
  
  BeginUndoAction();

  SetLineIndentation(currentLine, indent);
    
  if (level < m_FoldLevel && m_AddingChars)
  {
    SetLineIndentation(currentLine - 1, indent);
  }
  
  EndUndoAction();

  m_FoldLevel = level;
    
  GotoPos(GetLineIndentPosition(currentLine));

  return true;
}

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const wxString sel = GetSelectedText();

  if (GetCurrentLine() == 0 && !wxExLexers::Get()->GetLexers().empty())
  {
    menu.Append(ID_EDIT_SHOW_PROPERTIES, _("Properties"));
  }
    
  if (m_MenuFlags & STC_MENU_OPEN_LINK)
  {
    wxString filename;

    if (LinkOpen(false, &filename))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_LINK, _("Open") + " " + filename);
    }
    else if (LinkOpen(true))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_BROWSER, _("&Open In Browser"));
    }
  }

  if (m_MenuFlags & STC_MENU_VCS)
  {
    if (GetFileName().FileExists() && sel.empty())
    {
      if (wxExVCS::DirExists(GetFileName()))
      {
        menu.AppendSeparator();
        menu.AppendVCS(GetFileName());
      }
    }
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
      wxExMenu* menuSelection = new wxExMenu(menu.GetStyle());
      menuSelection->Append(ID_EDIT_UPPERCASE, _("&Uppercase\tF11"));
      menuSelection->Append(ID_EDIT_LOWERCASE, _("&Lowercase\tF12"));

      if (wxExGetNumberOfLines(sel) > 1)
      {
        wxExMenu* menuSort = new wxExMenu(menu.GetStyle());
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
  if (!m_UseAutoComplete || 
      !wxConfigBase::Get()->ReadBool(_("Auto complete"), true) ||
      SelectionIsRectangle())
  {
    return;
  }
  
  if (wxExIsCodewordSeparator(GetCharAt(GetCurrentPos() - 1)))
  {
    m_AutoComplete = c;
  }
  else
  {
    m_AutoComplete += c;

    if (m_AutoComplete.length() > 3) // Only autocompletion for large words
    {
      if (!AutoCompActive())
      {
        AutoCompSetIgnoreCase(true);
        AutoCompSetAutoHide(false);
      }

      if (m_Lexer.KeywordStartsWith(m_AutoComplete))
      {
        const wxString comp(
          m_Lexer.GetKeywordsString(-1, 5, m_AutoComplete));
          
        if (!comp.empty())
        {
          AutoCompShow(m_AutoComplete.length() - 1, comp);
        }
      }
      else
        AutoCompCancel();
    }
  }
}

void wxExSTC::CheckBrace()
{
  if (HexMode())
  {
    m_HexMode.HighlightOther();
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

void wxExSTC::Clear()
{
  if (m_vi.GetIsActive() && GetSelectedText().empty())
  {
    (void)m_vi.Command(std::string(1, WXK_DELETE));
  }
  else
  {
    wxStyledTextCtrl::Clear();
  }
}

void wxExSTC::ClearDocument(bool set_savepoint)
{
  SetReadOnly(false);
  
  ClearAll();
  
  if (set_savepoint)
  {
    EmptyUndoBuffer();
    SetSavePoint();
  }
}

// This is a static method, cannot use normal members here.
int wxExSTC::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  long flags,
  wxWindowID id)
{
  STCDefaults use;
  wxConfigBase* cfg = use.Get();
  
  static const std::vector<wxExItem> items {
    {"stc-notebook", {
      {_("General"),
        {{"stc-subnotebook", {
          {_("Page1"), 
            {{{_("End of line"),
               _("Line numbers"),
               _("Use tabs"),
               _("Caret line"),
               _("Scroll bars"),
               _("Auto complete"),
               _("vi mode")}}}},
          {_("Page2"), 
            {{_("Auto indent"), {
               {INDENT_NONE, _("None")},
               {INDENT_WHITESPACE, _("Whitespace")},
               {INDENT_LEVEL, _("Level")},
               {INDENT_ALL, _("Both")}}, true, 4},
             {_("Wrap visual flags"), {
               {wxSTC_WRAPVISUALFLAG_NONE, _("None")},
               {wxSTC_WRAPVISUALFLAG_END, _("End")},
               {wxSTC_WRAPVISUALFLAG_START, _("Start")},
               {wxSTC_WRAPVISUALFLAG_MARGIN, _("Margin")}}, true, 4},
             {_("Whitespace visible"), {
               {wxSTC_WS_INVISIBLE, _("Off")},
               {wxSTC_WS_VISIBLEAFTERINDENT, _("After indent")},
               {wxSTC_WS_VISIBLEALWAYS, _("Always")}
#if wxCHECK_VERSION(3,1,1)
               ,{wxSTC_WS_VISIBLEONLYININDENT, _("Only indent")}},
#else
               },
#endif  
               true, 4},
             {_("Wrap line"), {
               {wxSTC_WRAP_NONE, _("None")},
               {wxSTC_WRAP_WORD, _("Word")},
               {wxSTC_WRAP_CHAR, _("Char")}
#if wxCHECK_VERSION(3,1,0)
              ,{wxSTC_WRAP_WHITESPACE, _("Whitespace")}},
#else
               },
#endif  
              true, 4}}}} 
#ifdef __WXMSW__
            ,ITEM_NOTEBOOK_AUI
#endif
            }}},
      {_("Font"), 
#ifndef __WXOSX__
        {{_("Default font"), ITEM_FONTPICKERCTRL},
         {_("Tab font"), ITEM_FONTPICKERCTRL},
         {_("Text font"), ITEM_FONTPICKERCTRL}}},
#else
        {{_("Default font")},
         {_("Tab font")},
         {_("Text font")}}},
#endif
      {_("Edge"),
        {{_("Edge column"), 0, 500},
         { _("Edge line"), {
           {wxSTC_EDGE_NONE, _("None")},
           {wxSTC_EDGE_LINE, _("Line")},
           {wxSTC_EDGE_BACKGROUND, _("Background")}}, true, 1}}},
      {_("Margin"),
        {{_("Tab width"), 1, (int)cfg->ReadLong(_("Edge column"), 0)},
         {_("Indent"), 0, (int)cfg->ReadLong(_("Edge column"), 0)},
         {_("Divider"), 0, 40},
         {_("Folding"), 0, 40},
         {_("Line number"), 0, 100},
         {_("Auto complete maxwidth"), 0, 100}}},
      {_("Folding"),
        {{_("Indentation guide"), ITEM_CHECKBOX},
         {_("Auto fold"), 0, INT_MAX},
         {_("Fold flags"), {
             {wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")},
             {wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _("Line before contracted")},
             {wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")},
             {wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _("Line after contracted")},
             {wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")}},
             false}}},
      {_("Printer"),
        {{_("Print flags"), {
           {wxSTC_PRINT_NORMAL, _("Normal")},
           {wxSTC_PRINT_INVERTLIGHT, _("Invert on white")},
           {wxSTC_PRINT_BLACKONWHITE, _("Black on white")},
           {wxSTC_PRINT_COLOURONWHITE, _("Colour on white")},
           {wxSTC_PRINT_COLOURONWHITEDEFAULTBG, _("Colour on white normal")}}, true, 1}}},
      {_("Directory"),
        {{_("Include directory"), ITEM_LISTVIEW, wxAny(), false, wxID_ANY, LABEL_NONE}}}}}};

  int buttons = wxOK | wxCANCEL;

  if (flags & STC_CONFIG_WITH_APPLY)
  {
    buttons |= wxAPPLY;
  }
  
  if (!(flags & STC_CONFIG_MODELESS))
  {
    return wxExItemDialog(parent, items, 
      id == wxID_PREFERENCES ? wxGetStockLabel(id, 0): title, 0, 1, buttons, id).ShowModal();
  }
  else
  {
    if (m_ConfigDialog == nullptr)
    {
      m_ConfigDialog = new wxExItemDialog(parent, items, 
        id == wxID_PREFERENCES ? wxGetStockLabel(id, 0): title, 0, 1, buttons, id, 
        wxDefaultPosition, wxSize(510, 500));
    }

    return m_ConfigDialog->Show();
  }
}

void wxExSTC::ConfigGet(bool init)
{
  STCDefaults use;
  wxConfigBase* cfg = use.Get();
  
  const wxFont font(cfg->ReadObject(
    _("Default font"), wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT)));

  if (m_DefaultFont != font)
  {
    m_DefaultFont = font;
    
    StyleResetDefault();
    
    // Doing this once is enough, not yet possible.
    wxExLexers::Get()->LoadDocument();
    
    m_Lexer.Apply();
  }

  if (GetFileName().GetExt().CmpNoCase("log") == 0)
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }
  else
  {
    SetEdgeColumn(cfg->ReadLong(_("Edge column"), 0));
  
    const int el = cfg->ReadLong(_("Edge line"), wxSTC_EDGE_NONE);
  
    if (el != wxSTC_EDGE_NONE)
    {
      SetEdgeMode(font.IsFixedWidth() ? el: wxSTC_EDGE_BACKGROUND);
    }
    else
    {
      SetEdgeMode(el);
    }
  }
  
  if (init)
  {
    Fold();
  }

  AutoCompSetMaxWidth(cfg->ReadLong(_("Auto complete maxwidth"), 0));
  SetCaretLineVisible(cfg->ReadBool(_("Caret line"), true));
  SetFoldFlags(cfg->ReadLong( _("Fold flags"), 0));
  SetIndent(cfg->ReadLong(_("Indent"), 0));
  SetIndentationGuides( cfg->ReadBool(_("Indentation guide"), false));
  SetMarginWidth(m_MarginDividerNumber,  cfg->ReadLong(_("Divider"), 0));
  SetPrintColourMode(cfg->ReadLong(_("Print flags"), 0));
  SetTabWidth(cfg->ReadLong(_("Tab width"), 0));
  SetUseHorizontalScrollBar(cfg->ReadBool(_("Scroll bars"), true));
  SetUseTabs(cfg->ReadBool(_("Use tabs"), false));
  SetUseVerticalScrollBar(cfg->ReadBool(_("Scroll bars"), true));
  SetViewEOL(cfg->ReadBool(_("End of line"), false));
  SetViewWhiteSpace(cfg->ReadLong(_("Whitespace visible"), wxSTC_WS_INVISIBLE));
  SetWrapMode(cfg->ReadLong(_("Wrap line"), wxSTC_WRAP_NONE));
  SetWrapVisualFlags(cfg->ReadLong(_("Wrap visual flags"),  wxSTC_WRAPVISUALFLAG_END));
  m_vi.Use(cfg->ReadBool(_("vi mode"), false));
  
  ShowLineNumbers(cfg->ReadBool(_("Line numbers"), false));

  m_Link.SetFromConfig();
}

void wxExSTC::Copy()
{
  if (CanCopy()) 
  {
    wxStyledTextCtrl::Copy();
  }
}

void wxExSTC::Cut()
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
  
bool wxExSTC::FileReadOnlyAttributeChanged()
{
  SetReadOnly(!GetFileName().IsFileWritable()); // does not return anything
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
    GetFindString(),
    -1,
    find_next);
}

bool wxExSTC::FindNext(
  const wxString& text, 
  int find_flags,
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
  SetSearchFlags(find_flags);

  if (SearchInTarget(text) == -1)
  {
    wxExFrame::StatusText(
      wxExGetFindResult(text, find_next, recursive), wxEmptyString);
    
    bool found = false;
    
    if (!recursive)
    {
      recursive = true;
      found = FindNext(text, GetSearchFlags(), find_next);
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
      break;
      
      case wxExVi::MODE_VISUAL:
        SetSelection(GetSelectionStart(), GetTargetEnd());
      break;
      
      case wxExVi::MODE_VISUAL_LINE:
        SetSelection(
          PositionFromLine(LineFromPosition(GetSelectionStart())), 
          PositionFromLine(LineFromPosition(GetTargetEnd()) + 1));
      break;
      
      case wxExVi::MODE_VISUAL_RECT:
        while (GetCurrentPos() < GetTargetEnd())
        {
          CharRightRectExtend();
        }
      break;
    }
      
    EnsureVisible(LineFromPosition(GetTargetStart()));
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
      wxConfigBase::Get()->ReadLong(_("Folding"), 0));
    SetFoldFlags(
      wxConfigBase::Get()->ReadLong(_("Fold flags"),
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | 
        wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
        
    if (
      foldall || 
      GetLineCount() > wxConfigBase::Get()->ReadLong(_("Auto fold"), 0))
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
  const bool xml = (m_Lexer.GetLanguage() == "xml");

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


void wxExSTC::GotoLineAndSelect(
  int line_number, 
  const wxString& text,
  int col_number,
  long flags)
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
    const int start_pos = PositionFromLine(line_number - 1);
    const int end_pos = GetLineEndPosition(line_number - 1);

    SetSearchFlags(-1);
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
  
  if (flags & STC_WIN_FROM_OTHER)
  {
    IndicatorClearRange(0, GetTextLength() - 1);
    SetIndicator(wxExIndicator(0), 
      PositionFromLine(line_number - 1), 
      col_number > 0 ? 
        PositionFromLine(line_number - 1) + col_number - 1:
        GetLineEndPosition(line_number - 1));
  }
}

void wxExSTC::GuessType()
{
  // Get a small sample from this document to detect the file mode.
  const int length = (!HexMode() ? GetTextLength(): m_HexMode.GetBuffer().size());
  const int sample_size = (length > 255 ? 255: length);
  
  const wxString text = (!HexMode() ? GetTextRange(0, sample_size): 
    m_HexMode.GetBuffer().Mid(0, sample_size));

  std::vector<wxString> v;  
  
  // If we have a modeline comment.
  if (
    m_vi.GetIsActive() && 
    wxExMatch("vi: *(set [a-z0-9:=! ]+)", text.ToStdString(), v) > 0)
  {
    if (!m_vi.Command(wxString(":" + v[0]).ToStdString()))
    {
      wxLogStatus("Could not apply vi settings");
    }
  }

  if      (text.Contains("\r\n")) SetEOLMode(wxSTC_EOL_CRLF);
  else if (text.Contains("\n"))   SetEOLMode(wxSTC_EOL_LF);
  else if (text.Contains("\r"))   SetEOLMode(wxSTC_EOL_CR);
  else return; // do nothing

#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
}

void wxExSTC::Initialize(bool file_exists)
{
  if (wxConfig::Get()->ReadBool("AllowSync", true)) Sync();
  
  if (m_Flags & STC_WIN_HEX)
  {
    m_HexMode.Set(true);
  }

  m_AllowChangeIndicator = !(m_Flags & STC_WIN_NO_INDICATOR);
  
  if (!wxExLexers::Get()->GetLexers().empty())
  {
    m_DefaultFont = wxConfigBase::Get()->ReadObject(
      _("Default font"), wxSystemSettings::GetFont(wxSYS_ANSI_FIXED_FONT));
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
  SetMarginMask(m_MarginFoldingNumber, wxSTC_MASK_FOLDERS);
  SetMarginSensitive(m_MarginFoldingNumber, true);
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
  
  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    if (HexMode())
    {  
      if (
        event.GetKeyCode() == WXK_LEFT ||
        event.GetKeyCode() == WXK_RIGHT)
      {
        wxExHexModeLine(&m_HexMode).SetPos(event);
      }
    }
    if (m_vi.OnKeyDown(event))
    {
      event.Skip();
    }});

  Bind(wxEVT_KEY_UP, [=](wxKeyEvent& event) {
    event.Skip();
    CheckBrace();
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;});
      
  Bind(wxEVT_LEFT_DCLICK, [=](wxMouseEvent& event) {
    wxString filename;
    if (LinkOpen(false, &filename)) LinkOpen();
    else if (LinkOpen(true, &filename));
    else event.Skip();});
  
  Bind(wxEVT_LEFT_UP, [=](wxMouseEvent& event) {
    PropertiesMessage();
    event.Skip();
    CheckBrace();
    m_AddingChars = false;
    m_FoldLevel = 
      (GetFoldLevel(GetCurrentLine()) & wxSTC_FOLDLEVELNUMBERMASK) 
      - wxSTC_FOLDLEVELBASE;});
  
  if (m_MenuFlags != STC_MENU_NONE)
  {
    Bind(wxEVT_RIGHT_UP, [=](wxMouseEvent& event) {
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
      }});
  }
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    m_Frame->SetFindFocus(this);
    event.Skip();});

  Bind(wxEVT_STC_AUTOCOMP_SELECTION, [=](wxStyledTextEvent& event) {
    if (m_vi.GetIsActive())
    {
      const std::string command(wxString(
        event.GetText().Mid(m_AutoComplete.size())).ToStdString());
      if (!command.empty() && !m_vi.Command(command))
      {
        wxLogStatus("Autocomplete failed");
      }
    }});
    
  Bind(wxEVT_STC_CHARADDED, [=](wxStyledTextEvent& event) {
    event.Skip();
    AutoIndentation(event.GetKey());});
    
#if wxUSE_DRAG_AND_DROP
  Bind(wxEVT_STC_DO_DROP, [=](wxStyledTextEvent& event) {
    if (HexMode() || GetReadOnly())
    {
      event.SetDragResult(wxDragNone);
    }
    event.Skip();});

  Bind(wxEVT_STC_START_DRAG, [=](wxStyledTextEvent& event) {
    if (HexMode() || GetReadOnly())
    {
      event.SetDragAllowMove(false);
    }
    event.Skip();});
#endif    
    
  Bind(wxEVT_STC_DWELLEND, [=](wxStyledTextEvent& event) {
    if (CallTipActive())
    {
      CallTipCancel();
    }});

  // if we support automatic fold, this can be removed,
  // not yet possible for wx3.0. And add wxSTC_AUTOMATICFOLD_CLICK
  // to configdialog, and SetAutomaticFold.
  Bind(wxEVT_STC_MARGINCLICK, [=](wxStyledTextEvent& event) {
    if (event.GetMargin() == m_MarginFoldingNumber)
    {
      const int line = LineFromPosition(event.GetPosition());
      const int level = GetFoldLevel(line);
      if ((level & wxSTC_FOLDLEVELHEADERFLAG) > 0)
      {
        ToggleFold(line);
      }
    }});

  Bind(wxEVT_STC_UPDATEUI, [=](wxStyledTextEvent& event) {
    event.Skip();
    wxExFrame::UpdateStatusBar(this, "PaneInfo");});
    
  wxExFindReplaceData* frd = wxExFindReplaceData::Get();
  
  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    frd->SetFindString(frd->GetFindString());
    FindNext(frd->SearchDown());});
    
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    frd->SetFindString(frd->GetFindString());
    FindNext(frd->SearchDown());});

  Bind(wxEVT_FIND_REPLACE, [=](wxFindDialogEvent& event) {
    ReplaceNext(wxExFindReplaceData::Get()->SearchDown());});
    
  Bind(wxEVT_FIND_REPLACE_ALL, [=](wxFindDialogEvent& event) {
    ReplaceAll(frd->GetFindString(), frd->GetReplaceString());});
    
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
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
          if (wxExHexModeLine(&m_HexMode).Replace(event.GetUnicodeKey()))
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
    if (
      event.GetUnicodeKey() == '>' && 
      m_Lexer.GetScintillaLexer() == "hypertext")
     {
       const int match_pos = FindText(
         GetCurrentPos() - 1,
         PositionFromLine(GetCurrentLine()),
         "<");
       if (match_pos != wxSTC_INVALID_POSITION && GetCharAt(match_pos + 1) != '!')
       {
         const wxString match(GetWordAtPos(match_pos + 1));
         if (
           !match.StartsWith("/") &&
            GetCharAt(GetCurrentPos() - 2) != '/' &&
           (m_Lexer.GetLanguage() == "xml" || m_Lexer.IsKeyword(match)) &&
           !SelectionIsRectangle())
         {
           const wxString add("</" + match + ">");
           if (m_vi.GetIsActive())
           {
             const int esc = 27;
             if (
               !m_vi.Command(add.ToStdString()) ||
               !m_vi.Command(wxString(wxUniChar(esc)).ToStdString()) ||
               !m_vi.Command("%") ||
               !m_vi.Command("i"))
             {
               wxLogStatus("Autocomplete failed");
             }
           }
           else
           {
             InsertText(GetCurrentPos(), add);
           }
         }
       }
     }});
	  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Copy();}, wxID_COPY);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Cut();}, wxID_CUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Paste();}, wxID_PASTE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Undo();}, wxID_UNDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Redo();}, wxID_REDO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SelectAll();}, wxID_SELECTALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {if (!GetReadOnly() && !HexMode()) Clear();}, wxID_DELETE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (HexMode())
    {
      m_HexMode.GotoDialog();
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
        this)) > 0)
      {
        GotoLineAndSelect(val);
      }
    }
    return true;}, wxID_JUMP_TO);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); event.Skip();}, wxID_FIND);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); event.Skip();}, wxID_REPLACE);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LinkOpen();}, ID_EDIT_OPEN_LINK);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const wxString propnames(PropertyNames());
    wxString properties = (!propnames.empty() ? "[Current properties]\n": wxString());
    
    // Add current (global and lexer) properties.  
    for (const auto& it1 : wxExLexers::Get()->GetProperties())
    {
      properties += it1.GetName() + "=" + GetProperty(it1.GetName()) + "\n";
    }
    for (const auto& it2 : m_Lexer.GetProperties())
    {
      properties += it2.GetName() + "=" + GetProperty(it2.GetName()) + "\n";
    }
    // Add available properties.
    if (!propnames.empty())
    {
      properties += "\n[Available properties]\n";
      wxStringTokenizer tkz(propnames, "\n");
    
      while (tkz.HasMoreTokens())
      {
        const wxString prop(tkz.GetNextToken());
        const wxString description(DescribeProperty(prop));
        properties += prop + 
          (!GetProperty(prop).empty() ? "=" + GetProperty(prop): wxString()) + 
          (!description.empty() ? ": " + description: wxString()) + "\n";
      }
    }
    if (m_EntryDialog == nullptr)
    {
      m_EntryDialog = new wxExSTCEntryDialog(
        wxTheApp->GetTopWindow(), 
        _("Properties"), 
        properties, 
        wxEmptyString, 
        wxOK);
      m_EntryDialog->GetSTC()->GetLexer().Set("props");
    }
    else
    {
      m_EntryDialog->GetSTC()->SetText(properties);
    }
    m_EntryDialog->Show();}, ID_EDIT_SHOW_PROPERTIES);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetSelectedText().length() > 2) return;
    const wxString& caption = _("Enter Control Character");
    if (HexMode()) return m_HexMode.ControlCharDialog(caption);
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

    static int value = ' '; // don't use 0 as default as nullptr is not handled
    if (GetSelectedText().length() == 1) value = GetSelectedText().GetChar(0);
    int new_value;
    if ((new_value = (int)wxGetNumberFromUser(_("Input") + " 0 - 255:",
      wxEmptyString, caption, value, 0, 255, this)) < 0) return;

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
      buffer[1] = 0;

      if (m_vi.GetIsActive())
      {
        m_vi.Command(std::string(buffer, 2));
      }
      else
      {
        AddTextRaw(buffer, 1);
      }
      
      ProcessChar(new_value);
    }
    
    value = new_value;
    }, ID_EDIT_CONTROL_CHAR);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (CallTipActive()) CallTipCancel();
    
    const auto pos = GetCurrentPos();
    if (HexMode())
    {
      CallTipShow(pos, wxExHexModeLine(&m_HexMode).GetInfo());
      return;
    }

    const wxString word = (!GetSelectedText().empty() ? GetSelectedText() : GetWordAtPos(pos));
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
    }}, ID_EDIT_HEX_DEC_CALLTIP);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {LowerCase();}, ID_EDIT_LOWERCASE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {UpperCase();}, ID_EDIT_UPPERCASE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {FoldAll();}, ID_EDIT_FOLD_ALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);}, ID_EDIT_UNFOLD_ALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {Reload(m_Flags ^ STC_WIN_HEX);}, ID_EDIT_HEX);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(++m_Zoom);}, ID_EDIT_ZOOM_IN);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {SetZoom(--m_Zoom);}, ID_EDIT_ZOOM_OUT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); FindNext(true);}, ID_EDIT_FIND_NEXT);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {GetFindString(); FindNext(false);}, ID_EDIT_FIND_PREVIOUS);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxString filename; LinkOpen(true, &filename);}, ID_EDIT_OPEN_BROWSER);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    const int level = GetFoldLevel(GetCurrentLine());
    const int line_to_fold = (level & wxSTC_FOLDLEVELHEADERFLAG) ?
      GetCurrentLine(): GetFoldParent(GetCurrentLine());
    ToggleFold(line_to_fold);}, ID_EDIT_TOGGLE_FOLD);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExVCSExecute(m_Frame, event.GetId() - ID_EDIT_VCS_LOWEST - 1, 
      std::vector< wxString >{GetFileName().GetFullPath()});},
      ID_EDIT_VCS_LOWEST, ID_EDIT_VCS_HIGHEST);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (GetReadOnly())
    {
      wxLogStatus(_("Document is readonly"));
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
        int eol_mode = wxSTC_EOL_LF; // default ID_EDIT_EOL_UNIX
        
        switch (event.GetId())
        {
          case ID_EDIT_EOL_DOS: eol_mode = wxSTC_EOL_CRLF; break;
          case ID_EDIT_EOL_MAC: eol_mode = wxSTC_EOL_CR; break;
        }
    
        ConvertEOLs(eol_mode);
        SetEOLMode(eol_mode);
#if wxUSE_STATUSBAR
        wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
      }
    }}, ID_EDIT_EOL_DOS, ID_EDIT_EOL_MAC);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    int line = (event.GetId() == ID_EDIT_MARKER_NEXT ? 
      wxStyledTextCtrl::MarkerNext(GetCurrentLine() + 1, 0xFFFF):
      wxStyledTextCtrl::MarkerPrevious(GetCurrentLine() - 1, 0xFFFF));
    if (line == -1)
    {
      line = (event.GetId() == ID_EDIT_MARKER_NEXT ?
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
    }}, ID_EDIT_MARKER_NEXT, ID_EDIT_MARKER_PREVIOUS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    long pos;
    if ((pos = wxGetNumberFromUser(_("Input") + ":",
      wxEmptyString,
      _("Enter Sort Position"),
      GetCurrentPos() + 1 - PositionFromLine(GetCurrentLine()),
      1,
      GetLineEndPosition(GetCurrentLine()),
      this)) > 0)
    {
      wxExSortSelection(this, event.GetId() == wxID_SORT_ASCENDING ? STRING_SORT_ASCENDING: STRING_SORT_DESCENDING, pos - 1);
    }}, wxID_SORT_ASCENDING, wxID_SORT_DESCENDING);
}

bool wxExSTC::LinkOpen(bool browser, wxString* filename)
{
  const wxString sel = GetSelectedText();
  const wxString text = (!sel.empty() ? sel: GetCurLine());
  int line_no = (browser ? -1 :0);
  int col_no = 0;
  const wxString path = m_Link.GetPath(text, line_no, col_no);
  
  if (!path.empty())
  {
    if (filename == nullptr && !browser)
    {
      if (m_Frame != nullptr)
      {
        return m_Frame->OpenFile(
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
      if (filename != nullptr)
      {
        if (browser)
        {
          *filename = path;
          wxLaunchDefaultBrowser(path);
        }
        else
        {
          *filename = wxFileName(path).GetFullName();
        }
      }
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
  

void wxExSTC::OnIdle(wxIdleEvent& event)
{
  event.Skip();
  
  if (
    m_File.CheckSync() &&
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !(m_Flags & STC_WIN_READ_ONLY) &&
      GetFileName().GetStat().IsReadOnly() != GetReadOnly())
  {
    FileReadOnlyAttributeChanged();
  }
}

void wxExSTC::OnStyledText(wxStyledTextEvent& event)
{
  MarkModified(event); 
  event.Skip();
}

bool wxExSTC::Open(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags,
  const std::string& command)
{
  if (GetFileName() == filename && line_number > 0)
  {
    GotoLineAndSelect(line_number, match, col_number, flags);
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
      GotoLineAndSelect(line_number, match, col_number, flags);
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
        FindNext(match, 0);
      }
    }

    PropertiesMessage();
    
    success = true;
  }
  else
  {
    success = false;
  }
  
  if (success && m_Frame != nullptr)
  {
    m_Frame->SetRecentFile(filename.GetFullPath());
  }
  
  m_vi.Command(command);
  
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
void wxExSTC::PrintPreview(wxPreviewFrameModalityKind kind)
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

  frame->InitializeWithModality(kind);
  frame->Show();
}
#endif

void wxExSTC::PropertiesMessage(long flags)
{
  wxExLogStatus(GetFileName(), flags);
  
#if wxUSE_STATUSBAR
  if (flags != STAT_SYNC)
  {
    wxExFrame::UpdateStatusBar(this, "PaneFileType");
    wxExFrame::UpdateStatusBar(this, "PaneLexer");
    wxExFrame::UpdateStatusBar(this, "PaneMode");
  }
  
  wxExFrame::UpdateStatusBar(this, "PaneInfo");
#endif

  if (!(flags & STAT_SYNC) && m_Frame != nullptr)
  {
    const wxString file = GetName() + 
      (GetReadOnly() ? " [" + _("Readonly") + "]": wxString());
    
    if (file.empty())
    {
      m_Frame->SetTitle(wxTheApp->GetAppName());
    }
    else
    {
      m_Frame->SetTitle(file);
    }
  }
}

void wxExSTC::Reload(long flags)
{
  if (!m_HexMode.Set((flags & STC_WIN_HEX) > 0, 
     (flags & STC_WIN_HEX) ? GetTextRaw(): wxCharBuffer()))
  {
    return;
  }
  
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
  
  int selection_from_end = 0;

  if (SelectionIsRectangle() || wxExGetNumberOfLines(GetSelectedText()) > 1)
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
      wxExFindReplaceData::Get()->UseRegEx() ?
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
    -1,
    find_next);
}

bool wxExSTC::ReplaceNext(
  const wxString& find_text, 
  const wxString& replace_text,
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
    for (const auto& it : replace_text)
    {
      wxExHexModeLine(&m_HexMode, GetTargetStart()).Replace(it);
    }
  }
  else
  {
    wxExFindReplaceData::Get()->UseRegEx() ?
      ReplaceTargetRE(replace_text):
      ReplaceTarget(replace_text);
  }

  FindNext(find_text, find_flags, find_next);
  
  return true;
}

 
void wxExSTC::ResetMargins(long flags)
{
  if (flags & STC_MARGIN_FOLDING) SetMarginWidth(m_MarginFoldingNumber, 0);
  if (flags & STC_MARGIN_DIVIDER) SetMarginWidth(m_MarginLineNumber, 0);
  if (flags & STC_MARGIN_LINENUMBER) SetMarginWidth(m_MarginDividerNumber, 0);
}

void wxExSTC::SelectNone()
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

void wxExSTC::SetSearchFlags(int flags)
{
  if (flags == -1)
  {
    flags = 0;
    
    wxExFindReplaceData* frd = wxExFindReplaceData::Get();
    if (frd->UseRegEx()) 
    {
      flags |= wxSTC_FIND_REGEXP;
#if wxCHECK_VERSION(3,1,1)
      flags |= wxSTC_FIND_CXX11REGEX;
#endif
    }
    if (frd->MatchWord()) flags |= wxSTC_FIND_WHOLEWORD;
    if (frd->MatchCase()) flags |= wxSTC_FIND_MATCHCASE;
  }

  wxStyledTextCtrl::SetSearchFlags(flags);
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
  SetMarginWidth(m_MarginLineNumber, show ? wxConfigBase::Get()->ReadLong(_("Line number"), 0): 0);
}

void wxExSTC::Sync(bool start)
{
  // do not use ?, compile error for gcc, as Bind is void, Unbind is bool
  if (start)
    Bind(wxEVT_IDLE, &wxExSTC::OnIdle, this);
  else
    Unbind(wxEVT_IDLE, &wxExSTC::OnIdle, this);
}

void wxExSTC::Undo()
{
  wxStyledTextCtrl::Undo();
  m_HexMode.Undo();
}

void wxExSTC::UseAutoComplete(bool use)
{
  m_UseAutoComplete = use;
}

void wxExSTC::UseModificationMarkers(bool use)
{
  if (use)
    Bind(wxEVT_STC_MODIFIED, &wxExSTC::OnStyledText, this);
  else
    Unbind(wxEVT_STC_MODIFIED, &wxExSTC::OnStyledText, this);
}

void wxExSTC::WordLeftRectExtend() 
{
  const int repeat = GetCurrentPos() - WordStartPosition(GetCurrentPos(), false);
  
  for (int i = 0; i < repeat ; i++)
  {
    CharLeftRectExtend();
  }
}

void wxExSTC::WordRightRectExtend() 
{
  const int repeat = WordEndPosition(GetCurrentPos(), false) - GetCurrentPos();
  
  for (int i = 0; i < repeat; i++)
  {
    CharRightRectExtend();
  }
}
#endif // wxUSE_GUI
