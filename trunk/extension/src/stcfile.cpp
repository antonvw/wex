/******************************************************************************\
* File:          stcfile.cpp
* Purpose:       Implementation of class wxExSTC
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/stcfile.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI

const int SCI_ADDTEXT = 2001;
const int SCI_APPENDTEXT = 2282;

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset space_between_fields = 1;
const wxFileOffset start_hex_field = 10;

BEGIN_EVENT_TABLE(wxExSTC, wxExStyledTextCtrl)
  EVT_CHAR(wxExSTC::OnChar)
  EVT_IDLE(wxExSTC::OnIdle)
  EVT_KEY_DOWN(wxExSTC::OnKeyDown)
  EVT_KEY_UP(wxExSTC::OnKeyUp)
  EVT_LEFT_UP(wxExSTC::OnMouse)
  EVT_RIGHT_UP(wxExSTC::OnMouse)
  EVT_MENU(wxID_DELETE, wxExSTC::OnCommand)
  EVT_MENU(wxID_JUMP_TO, wxExSTC::OnCommand)
  EVT_MENU(wxID_SAVE, wxExSTC::OnCommand)
  EVT_MENU(wxID_SELECTALL, wxExSTC::OnCommand)
  EVT_MENU(wxID_SORT_ASCENDING, wxExSTC::OnCommand)
  EVT_MENU(wxID_SORT_DESCENDING, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_UNDO, wxID_REDO, wxExSTC::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, wxExSTC::OnCommand)
  EVT_STC_CHARADDED(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_DWELLEND(wxID_ANY, wxExSTC::OnStyledText)
//  EVT_STC_DWELLSTART(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MACRORECORD(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MARGINCLICK(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MODIFIED(wxID_ANY, wxExSTC::OnStyledText)
END_EVENT_TABLE()

wxExConfigDialog* wxExSTC::m_ConfigDialog = NULL;

wxExSTC::wxExSTC(wxWindow* parent,
  const wxString& value,
  long open_flags,
  const wxString& title,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExStyledTextCtrl(parent, id, pos, size, style)
  , m_FileSaveInMenu(false)
  , m_Flags(open_flags)
  , m_MenuFlags(menu_flags)
  , m_PreviousLength(0)
{
  SetName(title);

  Initialize();

  if (!value.empty())
  {
    if (m_Flags & STC_OPEN_HEX)
    {
      AddTextHexMode(0, value.c_str());
    }
    else
    {
      SetText(value);
    }

    if (m_Flags & STC_OPEN_READ_ONLY ||
        // At this moment we do not allow to write in hex mode.
        m_Flags & STC_OPEN_HEX)
    {
      SetReadOnly(true);
    }
  }
  else
  {
    SetLexer();
  }

  PropertiesMessage();
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
  : wxExStyledTextCtrl(parent, id, pos, size, style)
  , m_FileSaveInMenu(false)
  , m_Flags(0)
  , m_MenuFlags(menu_flags)
  , m_PreviousLength(0)
{
  Initialize();

  Open(filename, line_number, match, flags);
}

wxExSTC::wxExSTC(const wxExSTC& stc)
{
  wxExStyledTextCtrl::Create(stc.GetParent());

  // Do not yet set GetFileName(), this is done by Open.
  // And m_Macro is shared, so not necessary.
  m_FileSaveInMenu = stc.m_FileSaveInMenu;
  m_Flags = stc.m_Flags;
  m_PreviousLength = stc.m_PreviousLength;
  m_MenuFlags = stc.m_MenuFlags;

  Initialize();

  if (stc.GetFileName().IsOk())
  {
    Open(stc.GetFileName(), -1, wxEmptyString, m_Flags);
  }
}

wxExSTC::~wxExSTC()
{
  delete m_vi;
}

void wxExSTC::AddBasePathToPathList()
{
  // First find the base path, if this is not yet on the list, add it.
  const wxString basepath_text = "Basepath:";

  const int find = FindText(
    0,
    1000, // the max pos to look for, this seems enough
    basepath_text,
    wxSTC_FIND_WHOLEWORD);

  if (find == -1)
  {
    return;
  }

  const int line = LineFromPosition(find);
  const wxString basepath = GetTextRange(
    find + basepath_text.length() + 1,
    GetLineEndPosition(line) - 3);

  m_PathList.Add(basepath);
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

  SetGlobalStyles();

  // Do not show an edge, eol or whitespace in hex mode.
  SetEdgeMode(wxSTC_EDGE_NONE);
  SetViewEOL(false);
  SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  const wxFileOffset mid_in_hex_field = 7;

  wxString text;

  // Allocate space for the string.
  // Offset requires 10 * length / 16 bytes (+ 1 + 1 for separators, hex field 3 * length and the
  // ascii field just the length.
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

    text += wxString::Format("%08lx: ", (unsigned long)start + offset) +
      field_hex +
      wxString(' ', space_between_fields + ((bytes_per_line - count)* each_hex_field)) +
      field_ascii +
      GetEOL();
  }

  AddText(text);
}

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const wxString sel = GetSelectedText();
  const wxString link = GetTextAtCurrentPos();
  const int line_no = (!sel.empty() ? 
    wxExGetLineNumberFromText(sel): 
    GetLineNumberAtCurrentPos());

  if (m_MenuFlags & STC_MENU_OPEN_LINK)
  {
    wxString filename;
    if (LinkOpen(link, filename, line_no, false))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_LINK, _("Open") + " " + filename);
    }
  }

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
    else
    {
      if (m_MenuFlags & STC_MENU_INSERT)
      {
        menu.AppendSeparator();
        menu.Append(ID_EDIT_INSERT_SEQUENCE, wxExEllipsed(_("Insert Sequence")));
      }

      if (m_FileSaveInMenu && GetModify())
      {
        menu.AppendSeparator();
        menu.Append(wxID_SAVE);
      }
    }
  }

  // Folding if nothing selected, property is set,
  // and we have a lexer.
  if (
    sel.empty() && 
    GetProperty("fold") == "1" &&
   !GetFileName().GetLexer().GetScintillaLexer().empty())
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_TOGGLE_FOLD, _("&Toggle Fold\tCtrl-T"));
    menu.Append(ID_EDIT_FOLD_ALL, _("&Fold All Lines\tF9"));
    menu.Append(ID_EDIT_UNFOLD_ALL, _("&Unfold All Lines\tF10"));
  }

  if (sel.empty() && 
      (GetFileName().GetLexer().GetScintillaLexer() == "hypertext" ||
       GetFileName().GetLexer().GetScintillaLexer() == "xml"))
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_OPEN_BROWSER, _("&Open In Browser"));
  }

  if (!GetReadOnly() && (CanUndo() || CanRedo()))
  {
    menu.AppendSeparator();
    if (CanUndo()) menu.Append(wxID_UNDO);
    if (CanRedo()) menu.Append(wxID_REDO);
  }
}

bool wxExSTC::CheckAutoComp(const wxUniChar c)
{
  static wxString autoc;

  if (isspace(GetCharAt(GetCurrentPos() - 1)))
  {
    autoc = c;
  }
  else
  {
    autoc += c;

    if (autoc.length() >= 3) // Only autocompletion for large words
    {
      if (!AutoCompActive())
      {
        AutoCompSetIgnoreCase(true);
        AutoCompSetAutoHide(false);
      }

      if (GetFileName().GetLexer().KeywordStartsWith(autoc))
        AutoCompShow(
          autoc.length() - 1,
          GetFileName().GetLexer().GetKeywordsString());
      else
        AutoCompCancel();
    }
  }

  return AutoCompActive();
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
      PositionFromLine(LineFromPosition(pos)) + start_hex_field + each_hex_field * offset + space);
    return true;
  }
  else if (col >= start_hex_field)
  {
    if (GetCharAt(pos) != ' ')
    {
      int space = 0;

      if (col >= start_hex_field + space_between_fields + (bytes_per_line * each_hex_field) / 2)
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
  wxExFrame::StatusText(wxEmptyString, "PaneLines");
#endif
  EmptyUndoBuffer();
  SetSavePoint();
}

void wxExSTC::Colourise()
{
  GetFileName().GetLexer().ApplyKeywords(this);
  SetGlobalStyles();
  wxExLexers::Get()->ApplyProperties(this);
  wxExLexers::Get()->ApplyMarkers(this);
  GetFileName().GetLexer().ApplyProperties(this);
  SetFolding();
  GetFileName().GetLexer().Colourise(this);
}

// This is a static method, cannot use normal members here.
int wxExSTC::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  long flags,
  wxWindowID id)
{
  std::vector<wxExConfigItem> items;

  wxString page;

  if (flags & STC_CONFIG_SIMPLE)
  {
    page = wxEmptyString;
  }
  else
  {
    page = _("Setting");
  }

  items.push_back(wxExConfigItem(
    _("Tab width"), 1, (int)wxConfigBase::Get()->ReadLong(_("Edge column"), 80), page));
  items.push_back(wxExConfigItem(
    _("Indent"), 1, (int)wxConfigBase::Get()->ReadLong(_("Edge column"), 80), page));

  std::set<wxString> bchoices;
  bchoices.insert(_("End of line"));
  bchoices.insert(_("Line numbers"));
  bchoices.insert(_("Use tabs"));
  bchoices.insert(_("vi mode"));
  items.push_back(wxExConfigItem(bchoices, page));

  std::map<long, const wxString> choices;
  choices.insert(std::make_pair(wxSTC_WS_INVISIBLE, _("Invisible")));
  choices.insert(std::make_pair(wxSTC_WS_VISIBLEAFTERINDENT, _("Invisible after ident")));
  choices.insert(std::make_pair(wxSTC_WS_VISIBLEALWAYS, _("Visible always")));
  items.push_back(wxExConfigItem(_("Whitespace"), choices, true, page));

  std::map<long, const wxString> wchoices;
  wchoices.insert(std::make_pair(wxSTC_WRAP_NONE, _("None")));
  wchoices.insert(std::make_pair(wxSTC_WRAP_WORD, _("Word")));
  wchoices.insert(std::make_pair(wxSTC_WRAP_CHAR, _("Char")));
  items.push_back(wxExConfigItem(_("Wrap line"), wchoices, true, page));

  std::map<long, const wxString> vchoices;
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_NONE, _("None")));
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_END, _("End")));
  vchoices.insert(std::make_pair(wxSTC_WRAPVISUALFLAG_START, _("Start")));
  items.push_back(wxExConfigItem(_("Wrap visual flags"), vchoices, true, page));

  if (!(flags & STC_CONFIG_SIMPLE))
  {
    items.push_back(wxExConfigItem(_("Edge column"), 0, 500, _("Edge")));

    std::map<long, const wxString> echoices;
    echoices.insert(std::make_pair(wxSTC_EDGE_NONE, _("None")));
    echoices.insert(std::make_pair(wxSTC_EDGE_LINE, _("Line")));
    echoices.insert(std::make_pair(wxSTC_EDGE_BACKGROUND, _("Background")));
    items.push_back(wxExConfigItem(_("Edge line"), echoices, true, _("Edge")));

    items.push_back(wxExConfigItem(_("Auto fold"), 0, INT_MAX, _("Folding")));
    items.push_back(wxExConfigItem()); // spacer
    items.push_back(wxExConfigItem(_("Indentation guide"), CONFIG_CHECKBOX, _("Folding")));

    std::map<long, const wxString> fchoices;
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_BOX, _("Box")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _("Line before contracted")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _("Line after contracted")));
    fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")));
    items.push_back(wxExConfigItem(_("Fold flags"), fchoices, false, _("Folding")));

    items.push_back(wxExConfigItem(_("Calltip"), CONFIG_COLOUR, _("Colour")));
    items.push_back(wxExConfigItem(_("Edge colour"), CONFIG_COLOUR, _("Colour")));

    items.push_back(wxExConfigItem(_("Divider"), 0, 40, _("Margin")));
    items.push_back(wxExConfigItem(_("Folding"), 0, 40, _("Margin")));
    items.push_back(wxExConfigItem(_("Line number"), 0, 100, _("Margin")));

    items.push_back(wxExConfigItem(_("Include directory"), _("Directory"), wxTE_MULTILINE));
  }

  const wxSize size
#ifdef __WXMSW__
    (355, 300);
#else
    (500, 350);
#endif

  int buttons = wxOK | wxCANCEL;

  if (flags & STC_CONFIG_WITH_APPLY)
  {
    buttons |= wxAPPLY;
  }

  if (!(flags & STC_CONFIG_MODELESS))
  {
    wxExConfigDialog dlg(
      parent,
      items,
      title,
      0,
      2,
      buttons,
      id,
      wxDefaultPosition,
      size);

    return dlg.ShowModal();
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
        2,
        buttons,
        id,
        wxDefaultPosition,
        size);
    }

    return m_ConfigDialog->Show();
  }
}

void wxExSTC::ConfigGet()
{
  if (!wxConfigBase::Get()->Exists(_("Calltip")))
  {
    wxConfigBase::Get()->SetRecordDefaults(true);
  }

  CallTipSetBackground(wxConfigBase::Get()->ReadObject(
    _("Calltip"), wxColour("YELLOW")));

  SetEdgeColumn(wxConfigBase::Get()->ReadLong(_("Edge column"), 80));
  SetEdgeColour(wxConfigBase::Get()->ReadObject(
    _("Edge colour"), wxColour("GREY"))); 
  SetEdgeMode(wxConfigBase::Get()->ReadLong(_("Edge line"), wxSTC_EDGE_NONE));
  SetFoldFlags(wxConfigBase::Get()->ReadLong( _("Fold flags"),
    wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
  SetIndent(wxConfigBase::Get()->ReadLong(_("Indent"), 4));
  SetIndentationGuides(wxConfigBase::Get()->ReadBool(_("Indentation guide"), false));

  SetMarginWidth(m_MarginDividerNumber, wxConfigBase::Get()->ReadLong(_("Divider"), 16));
  SetMarginWidth(m_MarginFoldingNumber, wxConfigBase::Get()->ReadLong(_("Folding"), 16));

  const long margin = wxConfigBase::Get()->ReadLong(
    _("Line number"), 
    TextWidth(wxSTC_STYLE_DEFAULT, "999999"));

  SetMarginWidth(m_MarginLineNumber, (wxConfigBase::Get()->ReadBool(_("Line numbers"), false) ? margin: 0));

  SetTabWidth(wxConfigBase::Get()->ReadLong(_("Tab width"), 4));
  SetUseTabs(wxConfigBase::Get()->ReadBool(_("Use tabs"), false));
  SetViewEOL(wxConfigBase::Get()->ReadBool(_("End of line"), false));
  SetViewWhiteSpace(wxConfigBase::Get()->ReadLong(_("Whitespace"), wxSTC_WS_INVISIBLE));
  SetWrapMode(wxConfigBase::Get()->ReadLong(_("Wrap line"), wxSTC_WRAP_NONE));
  SetWrapVisualFlags(wxConfigBase::Get()->ReadLong(_("Wrap visual flags"), wxSTC_WRAPVISUALFLAG_END));

  m_viMode = wxConfigBase::Get()->ReadBool(_("vi mode"), false);

  wxStringTokenizer tkz(
    wxConfigBase::Get()->Read(_("Include directory")),
    ";");

  while (tkz.HasMoreTokens())
  {
    m_PathList.Add(tkz.GetNextToken());
  }

  if (wxConfigBase::Get()->IsRecordingDefaults())
  {
    // Set defaults only.
    wxConfigBase::Get()->ReadLong(_("Auto fold"), 2500);

    wxConfigBase::Get()->SetRecordDefaults(false);
  }
}

void wxExSTC::DoFileLoad(bool synced)
{
  // Synchronizing by appending only new data only works for log files.
  // Other kind of files might get new data anywhere inside the file,
  // we cannot sync that by keeping pos. Also only do it for reasonably large files,
  // so small log files are synced always (e.g. COM LIB report.log).
  const bool log_sync =
    synced &&
    GetFileName().GetExt().CmpNoCase("log") == 0 &&
    GetTextLength() > 1024;

  // Be sure we can add text.
  SetReadOnly(false);

  ReadFromFile(log_sync);

  if (!(m_Flags & STC_OPEN_HEX))
  {
    SetLexer();
  }
  else
  {
    wxExLexers::Get()->ApplyMarkers(this);
  }

  if (m_Flags & STC_OPEN_READ_ONLY ||
      GetFileName().GetStat().IsReadOnly() ||
      // At this moment we do not allow to write in hex mode.
      m_Flags & STC_OPEN_HEX)
  {
    SetReadOnly(true);
  }

  EmptyUndoBuffer();

  if (!synced)
  {
    const wxString msg = _("Opened") + ": " + GetFileName().GetFullPath();
    wxExLog::Get()->Log(msg);
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(msg);
#endif
    PropertiesMessage();
  }
  else
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(GetFileName(), wxExFrame::STAT_SYNC);
    UpdateStatusBar("PaneLines");
#endif
  }

  // No edges for log files.
  if (GetFileName().GetExt() == "log")
  {
    SetEdgeMode(wxSTC_EDGE_NONE);
  }

  if (GetFileName() == wxExLog::Get()->GetFileName())
  {
    DocumentEnd();
  }
}

void wxExSTC::DoFileSave(bool save_as)
{
  const wxCharBuffer& buffer = GetTextRaw(); 
  Write(buffer.data(), buffer.length());

  if (save_as)
  {
    Colourise();
  }

  const wxString msg = _("Saved") + ": " + GetFileName().GetFullPath();
  wxExLog::Get()->Log(msg);
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(msg);
#endif
}

void wxExSTC::EOLModeUpdate(int eol_mode)
{
  ConvertEOLs(eol_mode);
  SetEOLMode(eol_mode);
#if wxUSE_STATUSBAR
  UpdateStatusBar("PaneFileType");
#endif
}

void wxExSTC::FileNew(const wxExFileName& filename)
{
  wxExFile::FileNew(filename);

  PropertiesMessage();

  ClearDocument();

  SetLexer();
}

bool wxExSTC::FileReadOnlyAttributeChanged()
{
  if (!(m_Flags & STC_OPEN_HEX))
  {
    SetReadOnly(GetFileName().GetStat().IsReadOnly()); // does not return anything
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Readonly attribute changed"));
#endif
  }

  return true;
}

void wxExSTC::FileTypeMenu()
{
  if (GetReadOnly())
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Document is readonly"));
#endif
    return;
  }

  wxMenu* eol = new wxMenu();

  // The order here should be the same as the defines for wxSTC_EOL_CRLF.
  // So the FindItemByPosition can work
  eol->Append(ID_EDIT_EOL_DOS, "&DOS", wxEmptyString, wxITEM_CHECK);
  eol->Append(ID_EDIT_EOL_MAC, "&MAC", wxEmptyString, wxITEM_CHECK);
  eol->Append(ID_EDIT_EOL_UNIX, "&UNIX", wxEmptyString, wxITEM_CHECK);
  eol->FindItemByPosition(GetEOLMode())->Check();

  PopupMenu(eol);
}

void wxExSTC::GuessType()
{
  if (!(m_Flags & STC_OPEN_HEX))
  {
    // Get a small sample from this file to detect the file mode.
    const int sample_size = (GetTextLength() > 255 ? 255: GetTextLength());
    const wxString text = GetTextRange(0, sample_size);

    if      (text.Contains("\r\n")) SetEOLMode(wxSTC_EOL_CRLF);
    else if (text.Contains("\n"))   SetEOLMode(wxSTC_EOL_LF);
    else if (text.Contains("\r"))   SetEOLMode(wxSTC_EOL_CR);
    else return; // do nothing
  }

#if wxUSE_STATUSBAR
  UpdateStatusBar("PaneFileType");
#endif
}

void wxExSTC::Initialize()
{
  m_viMode = false;
  m_vi = new wxExVi(this);

  UsePopUp(false);
#ifdef __WXMSW__
  EOLModeUpdate(wxSTC_EOL_CRLF);
#else
  EOLModeUpdate(wxSTC_EOL_LF);
#endif
  ConfigGet();

  SetBackSpaceUnIndents(true);

  SetMouseDwellTime(1000);

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

  SetGlobalStyles();
}

void wxExSTC::LexerDialog(const wxString& caption)
{
  wxString lexer = GetFileName().GetLexer().GetScintillaLexer();

  if (wxExLexers::Get()->ShowDialog(this, lexer, caption))
  {
    SetLexer(lexer, true); // forced
  }
}

bool wxExSTC::LinkOpen(
  const wxString& link_with_line,
  wxString& filename,
  int line_number,
  bool open_link)
{
  wxString link;

  // Any line info is already in line_number if text was selected, so skip here.
  if (line_number != 0 && !GetSelectedText().empty())
    link = link_with_line.BeforeFirst(':');
  else
    link = link_with_line;

  if (link.empty())
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
      if (file.MakeAbsolute(GetFileName().GetPath()))
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

  if (!fullpath.empty() && open_link)
  {
    return Open(
      fullpath, 
      line_number, 
      wxEmptyString, 
      m_Flags | STC_OPEN_FROM_OTHER);
  }

  filename = wxFileName(fullpath).GetFullName();
  
  return !fullpath.empty();
}

void wxExSTC::OnChar(wxKeyEvent& event)
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
    CheckAutoComp(event.GetUnicodeKey());
  }
}

void wxExSTC::OnCommand(wxCommandEvent& command)
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
  case wxID_SAVE: FileSave(); break;
  case wxID_SORT_ASCENDING: SortSelectionDialog(true); break;
  case wxID_SORT_DESCENDING: SortSelectionDialog(false); break;

  case ID_EDIT_EOL_DOS: EOLModeUpdate(wxSTC_EOL_CRLF); break;
  case ID_EDIT_EOL_UNIX: EOLModeUpdate(wxSTC_EOL_LF); break;
  case ID_EDIT_EOL_MAC: EOLModeUpdate(wxSTC_EOL_CR); break;

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

  case ID_EDIT_OPEN_LINK:
    {
    const wxString sel = GetSelectedText();
    wxString filename;
    if (!sel.empty())
    {
      LinkOpen(sel, filename, wxExGetLineNumberFromText(sel));
    }
    else
    {
      LinkOpen(GetTextAtCurrentPos(), filename, GetLineNumberAtCurrentPos());
    }
    }
    break;
  case ID_EDIT_OPEN_BROWSER:
    if (GetModify())
    {
      FileSave();
    }

    wxLaunchDefaultBrowser(GetFileName().GetFullPath());
  break;

  case ID_EDIT_INSERT_SEQUENCE: SequenceDialog(); break;
  case ID_EDIT_LOWERCASE: LowerCase(); break;
  case ID_EDIT_UPPERCASE: UpperCase(); break;
  default: wxFAIL; break;
  }
}

void wxExSTC::OnIdle(wxIdleEvent& event)
{
  event.Skip();

  CheckFileSync();

  if (
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !(m_Flags & STC_OPEN_READ_ONLY) &&
    GetFileName().GetStat().IsOk() &&
    GetFileName().GetStat().IsReadOnly() != GetReadOnly())
  {
    FileReadOnlyAttributeChanged();
  }
}

void wxExSTC::OnKeyDown(wxKeyEvent& event)
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

void wxExSTC::OnKeyUp(wxKeyEvent& event)
{
  event.Skip();

  if (!CheckBrace(GetCurrentPos()))
  {
    if (!CheckBrace(GetCurrentPos() - 1))
    {
      if (m_Flags & STC_OPEN_HEX)
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
  else if (event.LeftUp())
  {
    PropertiesMessage();

    event.Skip();

    if (!CheckBrace(GetCurrentPos()))
    {
      CheckBrace(GetCurrentPos() - 1);
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

void wxExSTC::OnStyledText(wxStyledTextEvent& event)
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
  else if (event.GetEventType() == wxEVT_STC_MODIFIED)
  {
    // Only useful if this is not a file on disk, otherwise
    // the OnIdle already does this.
    if (!GetFileName().IsOk())
    {
#if wxUSE_STATUSBAR
      wxExFrame::StatusText(wxDateTime::Now().Format());
#endif
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

bool wxExSTC::Open(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  if (GetFileName() == filename && line_number > 0)
  {
    GotoLineAndSelect(line_number, match);
    PropertiesMessage();
    return true;
  }

  m_Flags = flags;

  if (wxExFile::FileLoad(filename))
  {
    // This should be after folding, and this one unfolds the line to go to.
    if (line_number > 0)
    {
      GotoLineAndSelect(line_number, match);
    }
    else
    {
      if (line_number == -1)
      {
        DocumentEnd();
      }
    }

    return true;
  }
  else
  {
    return false;
  }
}

void wxExSTC::PropertiesMessage() const
{
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(GetFileName());
  UpdateStatusBar("PaneFileType");
  UpdateStatusBar("PaneLexer");
  UpdateStatusBar("PaneLines");
#endif
}

void wxExSTC::ReadFromFile(bool get_only_new_data)
{
  const bool pos_at_end = (GetCurrentPos() >= GetTextLength() - 1);

  int startPos, endPos;
  GetSelection(&startPos, &endPos);

  wxFileOffset offset = 0;

  if (m_PreviousLength < Length() && get_only_new_data)
  {
    offset = m_PreviousLength;
  }

  if (offset == 0)
  {
    ClearDocument();
  }

  m_PreviousLength = Length();

  const wxCharBuffer& buffer = Read(offset);

  if (!(m_Flags & STC_OPEN_HEX))
  {
    // At least for toggling between hex and non-hex this is necessary to
    // reshow the edge line.
    ConfigGet();

    SetControlCharSymbol(0);

    const int message = (get_only_new_data ? SCI_APPENDTEXT: SCI_ADDTEXT);

    // README: The stc.h equivalents AddText, AddTextRaw, InsertText, InsertTextRaw do not add the length.
    // So for binary files this is the only way for opening.
    SendMsg(message, buffer.length(), (wxIntPtr)(const char *)buffer.data());
  }
  else
  {
    AddTextHexMode(offset, buffer);
  }

  if (get_only_new_data)
  {
    if (pos_at_end)
    {
      DocumentEnd();
    }
  }
  else
  {
    GuessType();
    DocumentStart();
  }

  if (startPos != endPos)
  {
    // TODO: This does not seem to work.
    SetSelection(startPos, endPos);
  }
}

void wxExSTC::ResetContentsChanged()
{
  SetSavePoint();
}

void wxExSTC::SetGlobalStyles()
{
  wxExLexers::Get()->GetDefaultStyle().Apply(this);

  StyleClearAll();

  if (!(m_Flags & STC_OPEN_HEX))
  {
    wxExLexers::Get()->ApplyGlobalStyles(this);
    wxExLexers::Get()->ApplyIndicators(this);
  }
  else
  {
    wxExLexers::Get()->ApplyHexStyles(this);
  }
}

void wxExSTC::SetLexer(const wxString& lexer, bool forced)
{
  ClearDocumentStyle();

  // Reset all old properties. 
  // Should be before SetFileNameLexer.
  GetFileName().GetLexer().ApplyResetProperties(this);

  SetFileNameLexer(lexer, (forced ? "forced": GetLine(0)));

#if wxUSE_STATUSBAR
  UpdateStatusBar("PaneLexer");
#endif

  // Update the lexer for scintilla.
  SetLexerLanguage(GetFileName().GetLexer().GetScintillaLexer());

  if (
    !GetFileName().GetLexer().GetScintillaLexer().empty() &&
    // And check whether the GetLexer from scintilla has a good value.
    // Otherwise it is not known, and we better show an error.
    wxStyledTextCtrl::GetLexer() == wxSTC_LEX_NULL)
  {
    wxLogError(_("Lexer is not known") + ": " + GetFileName().GetLexer().GetScintillaLexer());
  }

  Colourise();

  if (GetLineCount() > wxConfigBase::Get()->ReadLong(_("Auto fold"), -1))
  {
    FoldAll();
  }

  if (GetFileName().GetExt() == "po")
  {
    AddBasePathToPathList();
  }
}

void wxExSTC::SetText(const wxString& value)
{
  ClearDocument();

  // The stc.h equivalents SetText, AddText, AddTextRaw, InsertText, InsertTextRaw do not add the length.
  // So for text with nulls this is the only way for opening.
  SendMsg(SCI_ADDTEXT, value.length(), (wxIntPtr)(const char *)value.c_str());

  DocumentStart();

  ResetContentsChanged();

  // Do not allow the text specified to be undone.
  EmptyUndoBuffer();
}

#if wxUSE_STATUSBAR
void wxExSTC::UpdateStatusBar(const wxString& pane) const
{
  wxString text;

  if (pane == "PaneFileType")
  {
    if (m_Flags & STC_OPEN_HEX)
    {
      text = "HEX";
    }
    else
    {
      switch (GetEOLMode())
      {
      case wxSTC_EOL_CRLF: text = "DOS"; break;
      case wxSTC_EOL_CR: text = "MAC"; break;
      case wxSTC_EOL_LF: text = "UNIX"; break;
      default: text = "UNKNOWN";
      }
    }
  }
  else if (pane == "PaneLines")
  {
    if (GetCurrentPos() == 0) text = wxString::Format("%d", GetLineCount());
    else
    {
      int start;
      int end;
      const_cast< wxExSTC * >( this )->GetSelection(&start, &end);

      const int len  = end - start;
      const int line = 
        const_cast< wxExSTC * >( this )->GetCurrentLine() + 1;
      const int pos = GetCurrentPos() + 1 - PositionFromLine(line - 1);

      if (len == 0) text = wxString::Format("%d,%d", line, pos);
      else
      {
        // There might be NULL's inside selection.
        // So use the GetSelectedTextRaw variant.
        const int number_of_lines = wxExGetNumberOfLines(
          const_cast< wxExSTC * >( this )->GetSelectedTextRaw());
        if (number_of_lines <= 1) text = wxString::Format("%d,%d,%d", line, pos, len);
        else                      text = wxString::Format("%d,%d,%d", line, number_of_lines, len);
      }
    }
  }
  else if (pane == "PaneLexer")
  {
    text = GetFileName().GetLexer().GetScintillaLexer();
  }
  else
  {
    wxFAIL;
  }

  wxExFrame::StatusText(text, pane);
}
#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
