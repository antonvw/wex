////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wxExSTC
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/tokenzr.h>
#include <wx/extension/stc.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/defs.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

#if wxUSE_GUI

const int SCI_ADDTEXT = 2001;

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset space_between_fields = 1;
const wxFileOffset start_hex_field = 10;

BEGIN_EVENT_TABLE(wxExSTC, wxStyledTextCtrl)
  EVT_CHAR(wxExSTC::OnChar)
  EVT_FIND(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_FIND_REPLACE(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_FIND_REPLACE_ALL(wxID_ANY, wxExSTC::OnFindDialog)
  EVT_IDLE(wxExSTC::OnIdle)
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
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_UNDO, wxID_REDO, wxExSTC::OnCommand)
  EVT_RIGHT_UP(wxExSTC::OnMouse)
  EVT_STC_CHARADDED(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_DWELLEND(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MACRORECORD(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MARGINCLICK(wxID_ANY, wxExSTC::OnStyledText)
END_EVENT_TABLE()

std::vector <wxString> wxExSTC::m_Macro;
wxExConfigDialog* wxExSTC::m_ConfigDialog = NULL;
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
  , m_GotoLineNumber(1)
  , m_MarginDividerNumber(1)
  , m_MarginFoldingNumber(2)
  , m_MarginLineNumber(0)
  , m_MarkerChange(1)
  , m_vi(wxExVi(this))
  , m_File(this)
{
  Initialize();

  PropertiesMessage();

  if (!value.empty())
  {
    if (m_Flags & STC_WIN_HEX)
    {
      AddTextHexMode(0, value.c_str());
    }
    else
    {
      SetText(value);
    }

    GuessType();

    if (m_Flags & STC_WIN_READ_ONLY ||
        // At this moment we do not allow to write in hex mode.
        m_Flags & STC_WIN_HEX)
    {
      SetReadOnly(true);
    }
  }
}

wxExSTC::wxExSTC(wxWindow* parent,
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxStyledTextCtrl(parent, id, pos, size, style)
  , m_File(this)
  , m_GotoLineNumber(1)
  , m_MarginDividerNumber(1)
  , m_MarginFoldingNumber(2)
  , m_MarginLineNumber(0)
  , m_MarkerChange(1)
  , m_Flags(flags)
  , m_MenuFlags(menu_flags)
  , m_vi(wxExVi(this))
{
  Initialize();

  Open(filename, line_number, match, flags);
}

wxExSTC::wxExSTC(const wxExSTC& stc)
  : wxStyledTextCtrl(stc.GetParent())
  , m_Flags(stc.m_Flags)
  , m_GotoLineNumber(stc.m_GotoLineNumber)
  , m_MenuFlags(stc.m_MenuFlags)
  , m_MarginDividerNumber(stc.m_MarginDividerNumber)
  , m_MarginFoldingNumber(stc.m_MarginFoldingNumber)
  , m_MarginLineNumber(stc.m_MarginLineNumber)
  , m_MarkerChange(stc.m_MarkerChange)
  , m_vi(wxExVi(this)) // do not use stc.m_vi, crash
  , m_File(this)
{
  Initialize();

  if (stc.m_File.GetFileName().IsOk())
  {
    Open(stc.m_File.GetFileName(), -1, wxEmptyString, GetFlags());
  }
}

void wxExSTC::AddTextHexMode(wxFileOffset start, const wxCharBuffer& buffer)
/*
e.g.:
offset    hex field                                         ascii field
00000000: 23 69 6e 63 6c 75 64 65  20 3c 77 78 2f 63 6d 64  #include <wx/cmd
00000010: 6c 69 6e 65 2e 68 3e 20  2f 2f 20 66 6f 72 20 77  line.h> // for w
00000020: 78 43 6d 64 4c 69 6e 65  50 61 72 73 65 72 0a 23  xCmdLineParser #
          <----------------------------------------------> bytes_per_line
          <-> each_hex_field
                                     space_between_fields <>
                                  <- mid_in_hex_field
*/
{
  SetControlCharSymbol('x');
  wxExLexers::Get()->ApplyHexStyles(this);
  wxExLexers::Get()->ApplyMarkers(this);

  // Do not show an edge, eol or whitespace in hex mode.
  SetEdgeMode(wxSTC_EDGE_NONE);
  SetViewEOL(false);
  SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  const wxFileOffset mid_in_hex_field = 7;

  wxString text;

  // Allocate space for the string.
  // Offset requires 10 * length / 16 bytes (+ 1 + 1 for separators, 
  // hex field 3 * length and the ascii field just the length.
  text.Alloc(
    (start_hex_field + 1 + 1) * buffer.length() / bytes_per_line + 
     buffer.length() * each_hex_field + buffer.length());

  for (
    wxFileOffset offset = 0; 
    offset < buffer.length(); 
    offset += bytes_per_line)
  {
    long count = buffer.length() - offset;
    count =
      (bytes_per_line < count ? bytes_per_line : count);

    wxString field_hex, field_ascii;

    for (register wxFileOffset byte = 0; byte < count; byte++)
    {
      const char c = buffer.data()[offset + byte];

      field_hex += wxString::Format("%02x ", (unsigned char)c);

      // Print an extra space.
      if (byte == mid_in_hex_field)
      {
        field_hex += ' ';
      }

      // We do not want the \n etc. to be printed,
      // as that disturbs the hex view field.
      if (c != 0 && c != '\r' && c != '\n' && c != '\t')
      {
        field_ascii += c;
      }
      else
      {
        // Therefore print an ordinary ascii char.
        field_ascii += '.';
      }
    }

    // The extra space if we ended too soon.
    if (count <= mid_in_hex_field)
    {
      field_hex += ' ';
    }
    
    // Using wxString::Format here asserts (wxWidgets-2.9.1).
    char field_offset[11];
    sprintf(field_offset, "%08lx: ", (unsigned long)start + (unsigned long)offset);
    
    const wxString field_spaces = wxString(
        ' ', 
        space_between_fields + ((bytes_per_line - count)* each_hex_field));

    text +=  
      field_offset + 
      field_hex +
      field_spaces +
      field_ascii +
      GetEOL();
  }

  AddText(text);
}

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const wxString sel = GetSelectedText();

  if (m_MenuFlags & STC_MENU_OPEN_LINK)
  {
    const wxString link = GetTextAtCurrentPos();
    const int line_no = (!sel.empty() ? 
      wxExGetLineNumber(sel): 
      GetLineNumberAtCurrentPos());

    wxString filename;
    if (LinkOpen(link, line_no, &filename))
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
    else if (!wxConfigBase::Get()->Read(_("Comparator")).empty())
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_COMPARE, wxExEllipsed(_("&Compare Recent Version")));
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
      wxExMenu* menuSelection = menuSelection = new wxExMenu(menu);
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

void wxExSTC::CheckAutoComp(const wxUniChar& c)
{
  static wxString text;

  if (isspace(GetCharAt(GetCurrentPos() - 1)))
  {
    text = c;
  }
  else
  {
    text += c;

    if (text.length() >= 3) // Only autocompletion for large words
    {
      if (!AutoCompActive())
      {
        AutoCompSetIgnoreCase(true);
        AutoCompSetAutoHide(false);
      }

      if (m_Lexer.KeywordStartsWith(text))
        AutoCompShow(
          text.length() - 1,
          m_Lexer.GetKeywordsString());
      else
        AutoCompCancel();
    }
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
  const int col = GetColumn(pos);
  const wxFileOffset start_ascii_field =
    start_hex_field + each_hex_field * bytes_per_line + 2 * space_between_fields;

  if (col >= start_ascii_field)
  {
    const int offset = col - start_ascii_field;
    int space = 0;

    if (col >= start_ascii_field + bytes_per_line / 2)
    {
      space++;
    }

    BraceHighlight(pos,
      PositionFromLine(LineFromPosition(pos)) 
        + start_hex_field + each_hex_field * offset + space);
    return true;
  }
  else if (col >= start_hex_field)
  {
    if (GetCharAt(pos) != ' ')
    {
      int space = 0;

      if (col >= 
        start_hex_field + 
        space_between_fields + 
        (bytes_per_line * each_hex_field) / 2)
      {
        space++;
      }

      const int offset = (col - (start_hex_field + space)) / each_hex_field;

      BraceHighlight(pos,
        PositionFromLine(LineFromPosition(pos)) + start_ascii_field + offset);
      return true;
    }
  }
  else
  {
    BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
  }

  return false;
}

void wxExSTC::ClearDocument()
{
  SetReadOnly(false);
  ClearAll();
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(wxEmptyString, "PaneInfo");
#endif
  EmptyUndoBuffer();
  SetSavePoint();
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
  bchoices.insert(_("vi mode"));
  // use 2 cols here, but 1 for others on this page
  items.push_back(wxExConfigItem(bchoices, _("General"), 2)); 

  std::map<long, const wxString> choices;
  choices.insert(std::make_pair(wxSTC_WS_INVISIBLE, _("Invisible")));
  choices.insert(std::make_pair(wxSTC_WS_VISIBLEAFTERINDENT, 
    _("Invisible after ident")));
  choices.insert(std::make_pair(wxSTC_WS_VISIBLEALWAYS, _("Visible always")));
  items.push_back(wxExConfigItem(
    _("Whitespace"), 
    choices, 
    true, 
    _("General"),
    1));

  // Next code does not have any effect (2.9.1, on MSW and GTK)
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
    _("Wrap line"), 
    wchoices, 
    true,
    _("General"),
    1));

  std::map<long, const wxString> vchoices;
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_NONE, _("None")));
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_END, _("End")));
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_START, _("Start")));
  items.push_back(wxExConfigItem(
    _("Wrap visual flags"), 
    vchoices, 
    true, 
    _("General"),
    1));
    
  items.push_back(wxExConfigItem(_("Default font"), CONFIG_FONTPICKERCTRL));

  // Edge page.
  items.push_back(wxExConfigItem(_("Edge column"), 0, 500, _("Edge")));

  std::map<long, const wxString> echoices;
  echoices.insert(std::make_pair(wxSTC_EDGE_NONE, _("None")));
  echoices.insert(std::make_pair(wxSTC_EDGE_LINE, _("Line")));
  echoices.insert(std::make_pair(wxSTC_EDGE_BACKGROUND, _("Background")));
  items.push_back(wxExConfigItem(
    _("Edge line"), 
    echoices, 
    true, 
    _("Edge")));

  // Margin page.
  items.push_back(wxExConfigItem(
    _("Tab width"), 
    1, 
    (int)wxConfigBase::Get()->ReadLong(_("Edge column"), 80), _("Margin")));
  items.push_back(wxExConfigItem(
    _("Indent"), 
    1, 
    (int)wxConfigBase::Get()->ReadLong(_("Edge column"), 80), _("Margin")));
  items.push_back(
    wxExConfigItem(_("Divider"), 0, 40, _("Margin")));
  items.push_back(
    wxExConfigItem(_("Folding"), 0, 40, _("Margin")));
  items.push_back(
    wxExConfigItem(_("Line number"), 0, 100, _("Margin")));

  if (wxExLexers::Get()->Count() > 0)
  {
    // Folding page.
    items.push_back(wxExConfigItem(_("Auto fold"), 0, INT_MAX, _("Folding")));
    items.push_back(wxExConfigItem(_("Indentation guide"), CONFIG_CHECKBOX,
      _("Folding")));

    std::map<long, const wxString> fchoices;
    // next no longer available
//    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_BOX, _("Box")));
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

  if (!(flags & STC_CONFIG_SIMPLE))
  {
    // Directory page.
    items.push_back(
      wxExConfigItem(
        _("Include directory"), 
        CONFIG_LISTVIEW_FOLDER,
        _("Directory"), 
        false,
        wxID_ANY,
        25,
        false)); // no label
  }

  int buttons = wxOK | wxCANCEL;

  if (flags & STC_CONFIG_WITH_APPLY)
  {
    buttons |= wxAPPLY;
  }
  
#ifdef __WXOSX__
  // The listbook does not look nice on the iBook.
  const int style = wxExConfigDialog::CONFIG_NOTEBOOK;
#else  
  const int style = wxExConfigDialog::CONFIG_LISTBOOK;
#endif

  if (!(flags & STC_CONFIG_MODELESS))
  {
    return wxExConfigDialog(
      parent,
      items,
      title,
      0,
      1,
      buttons,
      id,
      style).ShowModal();
  }
  else
  {
    if (m_ConfigDialog == NULL)
    {
      m_ConfigDialog = new wxExConfigDialog(
        parent,
        items,
        title,
        0,
        1,
        buttons,
        id,
        style);
    }

    return m_ConfigDialog->Show();
  }
}

void wxExSTC::ConfigGet()
{
  if (!wxConfigBase::Get()->Exists(_("Caret line")))
  {
    wxConfigBase::Get()->SetRecordDefaults(true);
  }
  
  const wxFont font(wxConfigBase::Get()->ReadObject(
    _("Default font"), wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));

  if (m_DefaultFont != font)
  {
    m_DefaultFont = font;
    
    StyleResetDefault();
    
    // Doing this once is enough, not yet possible.
    wxExLexers::Get()->Read();
    
    SetLexer(GetLexer().GetScintillaLexer(), true);
  }

  const long def_tab_width = 2;

  if (m_File.GetFileName().GetExt().CmpNoCase("log") == 0)
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }
  else
  {
    SetEdgeColumn(wxConfigBase::Get()->ReadLong(_("Edge column"), 80));
    SetEdgeMode(wxConfigBase::Get()->ReadLong(_("Edge line"), wxSTC_EDGE_NONE));
  }
  
  SetCaretLineVisible(
    wxConfigBase::Get()->ReadBool(_("Caret line"), false));
    
  SetFoldFlags(wxConfigBase::Get()->ReadLong( _("Fold flags"),
    wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
  SetIndent(wxConfigBase::Get()->ReadLong(_("Indent"), def_tab_width));
  SetIndentationGuides(
    wxConfigBase::Get()->ReadBool(_("Indentation guide"), false));

  SetMarginWidth(
    m_MarginDividerNumber, 
    wxConfigBase::Get()->ReadLong(_("Divider"), 16));
    
  Fold();

  const int margin = wxConfigBase::Get()->ReadLong(
    _("Line number"), 
    TextWidth(wxSTC_STYLE_DEFAULT, "999999"));

  SetMarginWidth(
    m_MarginLineNumber, 
    (wxConfigBase::Get()->ReadBool(_("Line numbers"), false) ? margin: 0));

  // See above.
  /*
    SetSelectionMode(
    wxConfigBase::Get()->ReadLong(_("Selection mode"), wxSTC_SEL_STREAM));
  */
    
  SetTabWidth(wxConfigBase::Get()->ReadLong(_("Tab width"), def_tab_width));
  SetUseTabs(wxConfigBase::Get()->ReadBool(_("Use tabs"), false));
  SetViewEOL(wxConfigBase::Get()->ReadBool(_("End of line"), false));
  SetViewWhiteSpace(
    wxConfigBase::Get()->ReadLong(_("Whitespace"), wxSTC_WS_INVISIBLE));
  SetWrapMode(wxConfigBase::Get()->ReadLong(_("Wrap line"), wxSTC_WRAP_NONE));
  SetWrapVisualFlags(
    wxConfigBase::Get()->ReadLong(_("Wrap visual flags"), 
    wxSTC_WRAPVISUALFLAG_END));

  m_vi.Use(wxConfigBase::Get()->ReadBool(_("vi mode"), false));

  wxStringTokenizer tkz(
    wxConfigBase::Get()->Read(_("Include directory")),
    "\r\n");

  m_PathList.Empty();
  
  while (tkz.HasMoreTokens())
  {
    m_PathList.Add(tkz.GetNextToken());
  }

  if (wxConfigBase::Get()->IsRecordingDefaults())
  {
    // Set defaults only.
    wxConfigBase::Get()->ReadLong(_("Auto fold"), 1500);
    wxConfigBase::Get()->ReadLong(_("Folding"), 16);

    wxConfigBase::Get()->SetRecordDefaults(false);
  }
}

void wxExSTC::ControlCharDialog(const wxString& caption)
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
      wxMessageBox(
        wxString::Format("hex: %x dec: %d", value, value), 
        _("Control Character"));
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

void wxExSTC::Cut()
{
  wxStyledTextCtrl::Cut();
  
  MarkerAddChange(GetCurrentLine());
}
  
void wxExSTC::EOLModeUpdate(int eol_mode)
{
  if (GetReadOnly())
  {
    wxLogStatus(_("Document is readonly"));
    return;
  }

  ConvertEOLs(eol_mode);
  SetEOLMode(eol_mode);
#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
}

bool wxExSTC::FileReadOnlyAttributeChanged()
{
  if (!(GetFlags() & STC_WIN_HEX))
  {
    SetReadOnly(m_File.GetFileName().GetStat().IsReadOnly()); // does not return anything
    wxLogStatus(_("Readonly attribute changed"));
  }

  return true;
}

void wxExSTC::FileTypeMenu()
{
  wxMenu* menu = new wxMenu();

  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  menu->Append(ID_EDIT_EOL_DOS, "&DOS", wxEmptyString, wxITEM_CHECK);
  menu->Append(ID_EDIT_EOL_MAC, "&MAC", wxEmptyString, wxITEM_CHECK);
  menu->Append(ID_EDIT_EOL_UNIX, "&UNIX", wxEmptyString, wxITEM_CHECK);
  menu->AppendSeparator();
  wxMenuItem* hex = menu->Append(ID_EDIT_EOL_HEX, "&HEX", wxEmptyString, wxITEM_CHECK);
  
  if (!(GetFlags() & STC_WIN_HEX))
  {
    menu->FindItemByPosition(GetEOLMode())->Check();
  }
  else
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

  if (SearchInTarget(text) < 0)
  {
    wxExFindResult(text, find_next, recursive);
    
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
    wxLogStatus(wxEmptyString);
    
    recursive = false;

    if (GetTargetStart() != GetTargetEnd())
    {
      SetSelection(GetTargetStart(), GetTargetEnd());
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
  const bool xml = (m_Lexer.GetScintillaLexer() == "xml");

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

const wxString wxExSTC::GetFindString() const
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
        if (!isalnum(selection[i]))
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

int wxExSTC::GetLineNumberAtCurrentPos() const
{
  // This method is used by LinkOpen.
  // So, if no line number present return 0, 
  // otherwise link open jumps to last line.
  const int pos = GetCurrentPos();
  const int line_no = LineFromPosition(pos);

  // Cannot use GetLine, as that includes EOF, and then the ToLong does not
  // return correct number.
  const wxString text = const_cast< wxExSTC * >( this )->GetTextRange(
    PositionFromLine(line_no), 
    GetLineEndPosition(line_no));

  return wxExGetLineNumber(text);
}

const wxString wxExSTC::GetSelectedText() const
{
  // TODO: Fix crash for rectangular selection.
  if (SelectionIsRectangle())
  {
    return wxEmptyString;
  }
  else
  {
    return const_cast< wxExSTC * >( this )->wxStyledTextCtrl::GetSelectedText();
  }
}

const wxString wxExSTC::GetTextAtCurrentPos() const
{
  const wxString sel = GetSelectedText();

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
    if (pos_char1 == wxString::npos || 
        pos_char2 == wxString::npos || 
        pos_char2 <= pos_char1)
    {
      pos_char1 = text.find("<");
      pos_char2 = text.rfind(">");
    }

    // If that did not succeed, then get text between : and : (in .po files).
    if (pos_char1 == wxString::npos || 
        pos_char2 == wxString::npos || 
        pos_char2 <= pos_char1)
    {
      pos_char1 = text.find(": ");
      pos_char2 = text.rfind(":");
    }

    // If that did not succeed, then get text between ' and '.
    if (pos_char1 == wxString::npos ||
        pos_char2 == wxString::npos || 
        pos_char2 <= pos_char1)
    {
      pos_char1 = text.find("'");
      pos_char2 = text.rfind("'");
    }

    // If we did not find anything.
    if (pos_char1 == wxString::npos || 
        pos_char2 == wxString::npos || 
        pos_char2 <= pos_char1)
    {
      return wxEmptyString;
    }

    // Okay, get everything inbetween.
    const wxString match = 
      text.substr(pos_char1 + 1, pos_char2 - pos_char1 - 1);

    // And make sure we skip white space.
    return match.Strip(wxString::both);
  }
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

bool wxExSTC::GotoDialog(const wxString& caption)
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

  GotoLineAndSelect(val);

  return true;
}

void wxExSTC::GotoLineAndSelect(
  int line_number, 
  const wxString& text)
{
  // line_number and m_GotoLineNumber start with 1 and is allowed to be 
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

  m_GotoLineNumber = line_number;

  const int start_pos = PositionFromLine(line_number - 1);
  const int end_pos = GetLineEndPosition(line_number - 1);

  SetTargetStart(start_pos);
  SetTargetEnd(end_pos);

  if (!text.empty())
  {
    SetSearchFlags(wxExFindReplaceData::Get()->STCFlags());

    if (SearchInTarget(text) < 0)
    {
      bool recursive = true;
      wxExFindResult(text, true, recursive);
      return;
    }
  }

  SetSelection(GetTargetStart(), GetTargetEnd());
}

void wxExSTC::GuessType()
{
  if (!(GetFlags() & STC_WIN_HEX))
  {
    // Get a small sample from this file to detect the file mode.
    const int sample_size = (GetTextLength() > 255 ? 255: GetTextLength());
    const wxString text = GetTextRange(0, sample_size);
    const wxRegEx ex(".*vi: *set .*");
    
    if (ex.Matches(text))
    {
      m_vi.ExecCommand(text.AfterFirst(':').Trim(false));
    }

    if      (text.Contains("\r\n")) SetEOLMode(wxSTC_EOL_CRLF);
    else if (text.Contains("\n"))   SetEOLMode(wxSTC_EOL_LF);
    else if (text.Contains("\r"))   SetEOLMode(wxSTC_EOL_CR);
    else return; // do nothing
  }

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


void wxExSTC::Indent(int begin, int end, bool forward)
{
  BeginUndoAction();

  for (int i = 0; i <= end - begin; i++)
  {
    const int start = PositionFromLine(begin + i);

    if (forward)
    {
      if (GetUseTabs())
      {
        InsertText(start, '\t');
      }
      else
      {
        InsertText(start, wxString(' ', GetIndent()));
      }
    }
    else
    {
      const int cols = GetLineIndentation(begin + i);
      
      if (cols > 0)
      {
        if (GetUseTabs())
        {
          Remove(start, start + 1);
        }
        else
        {
          Remove(start, start + GetIndent());
        }
      }
    }
    
    MarkerAddChange(begin + i);
  }

  EndUndoAction();
}

void wxExSTC::Indent(int lines, bool forward)
{
  const int line = LineFromPosition(GetCurrentPos());

  Indent(line, line + lines - 1, forward);
}

void wxExSTC::Initialize()
{
  m_MacroIsRecording = false;
  m_SavedPos = 0;
  m_SavedSelectionStart = -1;
  m_SavedSelectionEnd = -1;
  
  m_DefaultFont = wxConfigBase::Get()->ReadObject(
    _("Default font"), wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT));
  
  Bind(
    wxEVT_STC_MODIFIED, 
    &wxExSTC::OnStyledText,
    this,
    wxID_ANY);

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

  const int accels = 19; // take max number of entries
  wxAcceleratorEntry entries[accels];

  int i = 0;

  entries[i++].Set(wxACCEL_CTRL, (int)'Z', wxID_UNDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'Y', wxID_REDO);
  entries[i++].Set(wxACCEL_CTRL, (int)'D', ID_EDIT_HEX_DEC_CALLTIP);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F3, ID_EDIT_FIND_NEXT);
  entries[i++].Set(wxACCEL_NORMAL, WXK_F4, ID_EDIT_FIND_PREVIOUS);
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
  entries[i++].Set(wxACCEL_CTRL, '=', ID_EDIT_ZOOM_IN);
  entries[i++].Set(wxACCEL_CTRL, '-', ID_EDIT_ZOOM_OUT);
  entries[i++].Set(wxACCEL_CTRL, '9', ID_EDIT_MARKER_NEXT);
  entries[i++].Set(wxACCEL_CTRL, '0', ID_EDIT_MARKER_PREVIOUS);

  wxAcceleratorTable accel(i, entries);
  SetAcceleratorTable(accel);
  
  wxExLexers::Get()->ApplyGlobalStyles(this);
  
  ConfigGet();
}

bool wxExSTC::IsTargetRE(const wxString& target) const
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

bool wxExSTC::LinkOpen(
  const wxString& link_with_line,
  int line_number,
  wxString* filename)
{
  // Any line info is already in line_number, so skip here.
  const wxString no = link_with_line.AfterFirst(':');
  const wxString link = (no.IsNumber() ? 
    link_with_line.BeforeFirst(':'): link_with_line);

  if (
    link.empty() || 
    // Otherwise, if you happen to select text that 
    // ends with a separator, wx asserts.
    wxFileName::IsPathSeparator(link.Last()))
  {
    return false;
  }

  wxFileName file(link);
  wxString fullpath;

  if (file.FileExists())
  {
    file.MakeAbsolute();
    fullpath = file.GetFullPath();
  }
  else
  {
    if (file.IsRelative())
    {
      if (file.MakeAbsolute(m_File.GetFileName().GetPath()))
      {
        if (file.FileExists())
        {
          fullpath = file.GetFullPath();
        }
      }
    }

    if (fullpath.empty())
    {
      fullpath = m_PathList.FindAbsoluteValidPath(link);
    }
  }
  
  if (!fullpath.empty())
  {
    if (filename == NULL)
    {
      return Open(
        fullpath, 
        line_number, 
        wxEmptyString, 
        GetFlags() | STC_WIN_FROM_OTHER);
    }
    else
    {
      *filename = wxFileName(fullpath).GetFullName();
    }
  }
  
  return !fullpath.empty();
}

void wxExSTC::MacroPlayback()
{
  wxASSERT(MacroIsRecorded());

  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_Macro.begin();
#else
    std::vector <wxString>::iterator it = m_Macro.begin();
#endif	
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

  wxLogStatus(_("Macro played back"));
}

void wxExSTC::MarkerAddChange(int line)
{
  if (
    !GetReadOnly() &&
     wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange) &&
     m_File.GetFileName().GetStat().IsOk())
  {
    MarkerAdd(line, m_MarkerChange.GetNo());
  }
}
  
void wxExSTC::MarkerDeleteAllChange()
{
  if (wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    MarkerDeleteAll(m_MarkerChange.GetNo());
  }
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
}
      
// cannot be const because of MarkerAddChange
void wxExSTC::MarkTargetChange()
{
  if (!wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    return;
  }
  
  const int line_begin = LineFromPosition(GetTargetStart());
  const int line_end = LineFromPosition(GetTargetEnd());
    
  for (int i = line_begin; i <= line_end; i++)
  {
    MarkerAddChange(i);
  }
}

void wxExSTC::OnChar(wxKeyEvent& event)
{
  const bool skip = m_vi.OnChar(event);

  if (skip && 
      GetReadOnly() && 
      wxIsalnum(event.GetUnicodeKey()))
  {
    wxLogStatus(_("Document is readonly"));
    return;
  }

  if (skip)
  {
    // Auto complete does not yet combine with vi mode.
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
  case wxID_DELETE: 
    if (!GetReadOnly()) 
    {
      Clear(); 
      MarkerAddChange(GetCurrentLine());
    }
    break;
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

  case ID_EDIT_COMPARE:
    {
    wxFileName lastfile;

    if (wxExFindOtherFileName(m_File.GetFileName(), &lastfile))
    {
      wxExCompareFile(m_File.GetFileName(), lastfile);
    }
    }
    break;

  case ID_EDIT_CONTROL_CHAR:
    ControlCharDialog();
  break;
  
  case ID_EDIT_EOL_DOS: EOLModeUpdate(wxSTC_EOL_CRLF); break;
  case ID_EDIT_EOL_UNIX: EOLModeUpdate(wxSTC_EOL_LF); break;
  case ID_EDIT_EOL_MAC: EOLModeUpdate(wxSTC_EOL_CR); break;
  case ID_EDIT_EOL_HEX: 
    {
    wxExFileDialog dlg(this, &m_File);
    if (dlg.ShowModalIfChanged() == wxID_CANCEL) return;
    Reload(m_Flags ^ STC_WIN_HEX); 
#if wxUSE_STATUSBAR
    wxExFrame::UpdateStatusBar(this, "PaneFileType");
#endif
    }
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

  case ID_EDIT_HEX_DEC_CALLTIP:
    HexDecCalltip(GetCurrentPos());
  break;

  case ID_EDIT_LOWERCASE: 
    LowerCase(); 
    TargetFromSelection();
    MarkTargetChange();
    break;
  case ID_EDIT_UPPERCASE: 
    UpperCase(); 
    TargetFromSelection();
    MarkTargetChange();
    break;
  
  case ID_EDIT_MARKER_NEXT: MarkerNext(true); break;
  case ID_EDIT_MARKER_PREVIOUS: MarkerNext(false); break;
  
  case ID_EDIT_OPEN_BROWSER:
    wxLaunchDefaultBrowser(m_File.GetFileName().GetFullPath());
    break;

  case ID_EDIT_OPEN_LINK:
    {
    const wxString sel = GetSelectedText();
    
    if (!sel.empty())
    {
      LinkOpen(sel, wxExGetLineNumber(sel));
    }
    else
    {
      LinkOpen(GetTextAtCurrentPos(), GetLineNumberAtCurrentPos());
    }
    }
    break;

  case ID_EDIT_READ: m_File.Read(command.GetString()); break;
    
  case ID_EDIT_ZOOM_IN:
    m_Zoom++;
    SetZoom(m_Zoom);
    break;

  case ID_EDIT_ZOOM_OUT:
    m_Zoom--;
    SetZoom(m_Zoom);
    break;

  case ID_EDIT_FIND_NEXT: 
  case ID_EDIT_FIND_PREVIOUS: 
    GetFindString();
    FindNext(command.GetId() == ID_EDIT_FIND_NEXT); 
    break;
    
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
    !(GetFlags() & STC_WIN_READ_ONLY) &&
    m_File.GetFileName().GetStat().IsOk() &&
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

  if (!m_vi.GetIsActive() || m_vi.OnKeyDown(event))
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

void wxExSTC::OnKeyUp(wxKeyEvent& event)
{
  event.Skip();

  if (!CheckBrace(GetCurrentPos()))
  {
    if (!CheckBrace(GetCurrentPos() - 1))
    {
      if (m_Flags & STC_WIN_HEX)
      {
        if (!CheckBraceHex(GetCurrentPos()))
        {
          CheckBraceHex(GetCurrentPos() - 1);
        }
      }
    }
  }
}

void wxExSTC::OnMouse(wxMouseEvent& event)
{
  if (event.LeftUp())
  {
    PropertiesMessage();

    event.Skip();

    if (!CheckBrace(GetCurrentPos()))
    {
      CheckBrace(GetCurrentPos() - 1);
    }
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
      if (GetReadOnly()) style |= wxExMenu::MENU_IS_READ_ONLY;
      if (!GetSelectedText().empty()) 
        style |= wxExMenu::MENU_IS_SELECTED;
      if (GetTextLength() == 0) style |= wxExMenu::MENU_IS_EMPTY;
      if (CanPaste()) style |= wxExMenu::MENU_CAN_PASTE;

      wxExMenu menu(style);
      
      BuildPopupMenu(menu);
      
      if (menu.GetMenuItemCount() > 0)
      {
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
    event.Skip();

//    MarkerAddChange(event.GetLine()); 
  }
  else if (event.GetEventType() == wxEVT_STC_DWELLEND)
  {
    if (CallTipActive())
    {
      CallTipCancel();
    }
  }
  else if (event.GetEventType() == wxEVT_STC_MACRORECORD)
  {
    wxString msg = wxString::Format("%d %d ", 
      event.GetMessage(), 
      event.GetWParam());

    if (event.GetLParam() != 0)
    {
      char* txt = (char *)(wxIntPtr)event.GetLParam();
      msg += txt;
    }
    else
    {
      msg += "0";
    }

    m_Macro.push_back(msg);
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
    MarkerAddChange(GetCurrentLine());
  }
  else if (event.GetEventType() == wxEVT_STC_MODIFIED)
  {
    event.Skip();

//    MarkerAddChange(event.GetLine());
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
  long flags)
{
  if (m_File.GetFileName() == filename && line_number > 0)
  {
    GotoLineAndSelect(line_number, match);
    PropertiesMessage();
    return true;
  }

  m_Flags = flags;

  Unbind(
    wxEVT_STC_MODIFIED, 
    &wxExSTC::OnStyledText,
    this,
    wxID_ANY);
    
  bool success;

  if (m_File.FileLoad(filename))
  {
    SetName(filename.GetFullPath());

    if (line_number > 0)
    {
      GotoLineAndSelect(line_number, match);
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

    success = true;
  }
  else
  {
    wxExFrame::StatusText(wxEmptyString, "PaneInfo");
    success = false;
  }
  
  Bind(
    wxEVT_STC_MODIFIED, 
    &wxExSTC::OnStyledText,
    this,
    wxID_ANY);

  return success;
}

void wxExSTC::Paste()
{
  const int line = GetCurrentLine();

  wxStyledTextCtrl::Paste();
  
  if (wxExLexers::Get()->MarkerIsLoaded(m_MarkerChange))
  {
    for (int i = line; i <= GetCurrentLine(); i++)
    {
      MarkerAddChange(i);
    }
  }
}

void wxExSTC::PositionRestore()
{
  if (m_SavedSelectionStart != -1 && m_SavedSelectionEnd != -1)
  {
    SetSelection(m_SavedSelectionStart, m_SavedSelectionEnd);
  }
  else
  {
    SetSelection(m_SavedPos, m_SavedPos);
  }
  
  SetCurrentPos(m_SavedPos);
  EnsureCaretVisible();
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
  if ((flags & STC_WIN_HEX) && 
     !(m_Flags & STC_WIN_HEX))
  {
    const wxCharBuffer buffer = GetTextRaw();
    ClearDocument();
    AddTextHexMode(0, buffer);
    SetReadOnly(true);
    m_Flags = flags;
    EmptyUndoBuffer();
    SetSavePoint();
  }
  else
  {
    Open(m_File.GetFileName(), 0, wxEmptyString, flags);
  }
}

int wxExSTC::ReplaceAll(
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

  SetSearchFlags(wxExFindReplaceData::Get()->STCFlags());
  int nr_replacements = 0;

  BeginUndoAction();

  const bool is_re = IsTargetRE(replace_text);

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
      MarkTargetChange();
  
      length = (is_re ?
        ReplaceTargetRE(replace_text):
        ReplaceTarget(replace_text));

      nr_replacements++;
    }

    SetTargetStart(target_start + length);
    SetTargetEnd(GetLength() - selection_from_end);
  }

  EndUndoAction();

  wxLogStatus(wxString::Format(_("Replaced: %d occurrences of: %s"),
    nr_replacements, find_text.c_str()));

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

  MarkTargetChange();
      
  IsTargetRE(replace_text) ?
    ReplaceTargetRE(replace_text):
    ReplaceTarget(replace_text);

  FindNext(find_text, search_flags, find_next);
  
  return true;
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

bool wxExSTC::SetLexer(const wxString& lexer, bool fold)
{
  if (!m_Lexer.ApplyLexer(lexer, this, false, fold))
  {
    return false;
  }
  
  if (fold)
  {
    Fold();
  }
  
  if (lexer == "diff")
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }
  
  wxExFrame::StatusText(m_Lexer.GetScintillaLexer(), "PaneLexer");
    
  return true;
}

void wxExSTC::SetLexerProperty(const wxString& name, const wxString& value)
{
  m_Lexer.SetProperty(name, value);
  m_Lexer.Apply(this);
}

void wxExSTC::SetText(const wxString& value)
{
  ClearDocument();

  // The stc.h equivalents SetText, AddText, AddTextRaw, InsertText, 
  // InsertTextRaw do not add the length.
  // So for text with nulls this is the only way for opening.
  SendMsg(SCI_ADDTEXT, value.length(), (wxIntPtr)(const char *)value.c_str());

  DocumentStart();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

bool wxExSTC::SmartIndentation()
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

    while (
      i < line.size() && 
      (line[i] == wxUniChar('\t') || line[i] == wxUniChar(' ')))
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
    for (
      std::multimap<wxString, wxString>::iterator it = mm.begin();
      it != mm.end();
      ++it)
    {
      text += it->second;
    }
  }
  else
  {
    for (
#ifdef wxExUSE_CPP0X	
      auto it = mm.rbegin();
#else
      std::multimap<wxString, wxString>::reverse_iterator it = mm.rbegin();
#endif	  
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

void wxExSTC::StartRecord()
{
  wxASSERT(!m_MacroIsRecording);

  m_MacroIsRecording = true;

  m_Macro.clear();

  wxLogStatus(_("Macro recording"));

  wxStyledTextCtrl::StartRecord();
}

void wxExSTC::StopRecord()
{
  wxASSERT(m_MacroIsRecording);

  m_MacroIsRecording = false;

  if (!m_Macro.empty())
  {
    wxLogStatus(_("Macro is recorded"));
  }

  wxStyledTextCtrl::StopRecord();
}

#endif // wxUSE_GUI
