/******************************************************************************\
* File:          stcfile.cpp
* Purpose:       Implementation of class wxExSTCFile
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

#if wxUSE_GUI

const int SCI_ADDTEXT = 2001;
const int SCI_APPENDTEXT = 2282;

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset space_between_fields = 1;
const wxFileOffset start_hex_field = 10;

BEGIN_EVENT_TABLE(wxExSTCFile, wxExSTC)
  EVT_IDLE(wxExSTCFile::OnIdle)
  EVT_KEY_UP(wxExSTCFile::OnKeyUp)
  EVT_LEFT_UP(wxExSTCFile::OnMouse)
  EVT_MENU(ID_EDIT_OPEN_LINK, wxExSTCFile::OnCommand)
  EVT_MENU(ID_EDIT_OPEN_BROWSER, wxExSTCFile::OnCommand)
  EVT_MENU(ID_EDIT_EOL_DOS, wxExSTCFile::OnCommand)
  EVT_MENU(ID_EDIT_EOL_UNIX, wxExSTCFile::OnCommand)
  EVT_MENU(ID_EDIT_EOL_MAC, wxExSTCFile::OnCommand)
  EVT_STC_MODIFIED(wxID_ANY, wxExSTCFile::OnStyledText)
END_EVENT_TABLE()

wxExConfigDialog* wxExSTCFile::m_ConfigDialog = NULL;

wxExSTCFile::wxExSTCFile(wxWindow* parent,
  const wxString& value,
  long open_flags,
  const wxString& title,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExSTC(parent, menu_flags, id, pos, size, style)
  , m_FileSaveInMenu(false)
  , m_Flags(open_flags)
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

  PropertiesMessage();
}

wxExSTCFile::wxExSTCFile(wxWindow* parent,
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExSTC(parent, menu_flags, id, pos, size, style)
  , m_FileSaveInMenu(false)
  , m_Flags(0)
  , m_PreviousLength(0)
{
  Initialize();

  Open(filename, line_number, match, flags);
}

wxExSTCFile::wxExSTCFile(const wxExSTCFile& stc)
  : wxExSTC(stc)
  , m_FileSaveInMenu(stc.m_FileSaveInMenu)
  , m_Flags(stc.m_Flags)
  , m_PreviousLength(stc.m_PreviousLength)
{
  Initialize();

  // Do not yet set GetFileName(), this is done by Open.
  if (stc.GetFileName().IsOk())
  {
    Open(stc.GetFileName(), -1, wxEmptyString, m_Flags);
  }
}

void wxExSTCFile::AddBasePathToPathList()
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

void wxExSTCFile::AddTextHexMode(wxFileOffset start, const wxCharBuffer& buffer)
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
  wxExLexers::Get()->ApplyHexStyles(this);
  wxExLexers::Get()->ApplyMarkers(this);

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

void wxExSTCFile::BuildPopupMenu(wxExMenu& menu)
{
  const wxString sel = GetSelectedText();

  if (GetMenuFlags() & STC_MENU_OPEN_LINK)
  {
    const wxString link = GetTextAtCurrentPos();
    const int line_no = (!sel.empty() ? 
      wxExGetLineNumberFromText(sel): 
      GetLineNumberAtCurrentPos());

    wxString filename;
    if (LinkOpen(link, filename, line_no, false))
    {
      menu.AppendSeparator();
      menu.Append(ID_EDIT_OPEN_LINK, _("Open") + " " + filename);
    }
  }

  wxExSTC::BuildPopupMenu(menu);

  if (
    !GetReadOnly() && 
     sel.empty() && 
     m_FileSaveInMenu && 
     GetModify())
  {
    menu.AppendSeparator();
    menu.Append(wxID_SAVE);
  }
}

bool wxExSTCFile::CheckBraceHex(int pos)
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

// This is a static method, cannot use normal members here.
int wxExSTCFile::ConfigDialog(
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
    // next is experimental, wait for scintilla
    //fchoices.insert(std::make_pair(wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")));
    items.push_back(wxExConfigItem(_("Fold flags"), fchoices, false, _("Folding")));

    items.push_back(wxExConfigItem(_("Calltip"), CONFIG_COLOUR, _("Colour")));
    items.push_back(wxExConfigItem(_("Edge colour"), CONFIG_COLOUR, _("Colour")));

    items.push_back(wxExConfigItem(_("Divider"), 0, 40, _("Margin")));
    items.push_back(wxExConfigItem(_("Folding"), 0, 40, _("Margin")));
    items.push_back(wxExConfigItem(_("Line number"), 0, 100, _("Margin")));

    items.push_back(wxExConfigItem(_("Include directory"), _("Directory"), wxTE_MULTILINE));
  }

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
      id);

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
        id);
    }

    return m_ConfigDialog->Show();
  }
}

void wxExSTCFile::ConfigGet()
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

  SetViMode(wxConfigBase::Get()->ReadBool(_("vi mode"), false));

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

void wxExSTCFile::DoFileLoad(bool synced)
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
    SetLexer(GetFileName().GetLexer().GetScintillaLexer());

    if (GetLexer().GetScintillaLexer().empty())
    {
      SetLexerByText();
    }

    if (GetFileName().GetExt() == "po")
    {
      AddBasePathToPathList();
    }
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
    wxExLog::Get()->Log(_("Opened") + ": " + GetFileName().GetFullPath());
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

void wxExSTCFile::DoFileSave(bool save_as)
{
  const wxCharBuffer& buffer = GetTextRaw(); 
  Write(buffer.data(), buffer.length());

  if (save_as)
  {
    SetName(GetFileName().GetFullPath());
    Colourise();
  }

  const wxString msg = _("Saved") + ": " + GetFileName().GetFullPath();
  wxExLog::Get()->Log(msg);
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(msg);
#endif
}

void wxExSTCFile::EOLModeUpdate(int eol_mode)
{
  ConvertEOLs(eol_mode);
  SetEOLMode(eol_mode);
#if wxUSE_STATUSBAR
  UpdateStatusBar("PaneFileType");
#endif
}

void wxExSTCFile::FileNew(const wxExFileName& filename)
{
  wxExFile::FileNew(filename);

  SetName(filename.GetFullPath());

  PropertiesMessage();

  ClearDocument();

  SetLexer(filename.GetLexer().GetScintillaLexer());
}

bool wxExSTCFile::FileReadOnlyAttributeChanged()
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

void wxExSTCFile::FileTypeMenu()
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

void wxExSTCFile::GuessType()
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

void wxExSTCFile::Initialize()
{
  ConfigGet();
  SetGlobalStyles();
}

bool wxExSTCFile::LinkOpen(
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

void wxExSTCFile::OnCommand(wxCommandEvent& command)
{
  switch (command.GetId())
  {
  case wxID_SAVE: FileSave(); break;

  case ID_EDIT_EOL_DOS: EOLModeUpdate(wxSTC_EOL_CRLF); break;
  case ID_EDIT_EOL_UNIX: EOLModeUpdate(wxSTC_EOL_LF); break;
  case ID_EDIT_EOL_MAC: EOLModeUpdate(wxSTC_EOL_CR); break;

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

  default: wxFAIL; break;
  }
}

void wxExSTCFile::OnIdle(wxIdleEvent& event)
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

void wxExSTCFile::OnKeyUp(wxKeyEvent& event)
{
  event.Skip();

  if (m_Flags & STC_OPEN_HEX)
  {
    if (!CheckBraceHex(GetCurrentPos()))
    {
      CheckBraceHex(GetCurrentPos() - 1);
    }
  }
}

void wxExSTCFile::OnMouse(wxMouseEvent& event)
{
  PropertiesMessage();
  event.Skip();
}

void wxExSTCFile::OnStyledText(wxStyledTextEvent& event)
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

bool wxExSTCFile::Open(
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
    SetName(filename.GetFullPath());

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

void wxExSTCFile::PropertiesMessage() const
{
#if wxUSE_STATUSBAR
  wxExFrame::StatusText(GetFileName());
  UpdateStatusBar("PaneFileType");
  UpdateStatusBar("PaneLexer");
  UpdateStatusBar("PaneLines");
#endif
}

void wxExSTCFile::ReadFromFile(bool get_only_new_data)
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

void wxExSTCFile::ResetContentsChanged()
{
  SetSavePoint();
}

#if wxUSE_STATUSBAR
void wxExSTCFile::UpdateStatusBar(const wxString& pane) const
{
  if (pane == "PaneFileType")
  {
    wxString text;

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

    wxExFrame::StatusText(text, pane);
  }
  else
  {
    wxExSTC::UpdateStatusBar(pane);
  }
}
#endif // wxUSE_STATUSBAR
#endif // wxUSE_GUI
