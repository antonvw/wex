/******************************************************************************\
* File:          stc.cpp
* Purpose:       Implementation of class wxExSTC and related classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/numdlg.h>
#include <wx/tokenzr.h>
#include <wx/extension/stc.h>
#include <wx/extension/app.h> // for wxExApp
#include <wx/extension/configdlg.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/textfile.h>

#if wxUSE_GUI

using namespace std;

const int SCI_ADDTEXT = 2001;
const int SCI_APPENDTEXT = 2282;

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset space_between_fields = 1;
const wxFileOffset start_hex_field = 10;

#if wxUSE_PRINTING_ARCHITECTURE
// Offers a print out to be used by wxExSTC.
class wxExPrintout : public wxPrintout
{
public:
  wxExPrintout(wxExSTC* owner);
private:
  void CountPages();
  void GetPageInfo(int* minPage, int* maxPage, int* pageFrom, int* pageTo);
  bool HasPage(int pageNum) {
    return (pageNum >= 1 && pageNum <= (int)m_PageBreaks.size());};
  void OnPreparePrinting();
  bool OnPrintPage(int pageNum);
  void SetScale(wxDC *dc);
  wxRect m_PageRect, m_PrintRect;
  int m_CurrentPage;
  vector<int> m_PageBreaks;
  wxExSTC* m_Owner;
};

wxExPrintout::wxExPrintout(wxExSTC* owner)
: wxPrintout(wxExPrintCaption(owner->GetFileName()))
  , m_PageRect()
  , m_PrintRect()
  , m_PageBreaks()
  , m_Owner(owner)
{
}

void wxExPrintout::CountPages()
{
  wxASSERT(GetDC() != NULL);

  wxBusyCursor wait;

  m_PageBreaks.clear();
  m_PageBreaks.push_back(0); // a page break at pos 0
  int pos = 0;

  while (pos < m_Owner->GetLength())
  {
    SetScale(GetDC());

    pos = m_Owner->FormatRange(
      false,
      pos,
      m_Owner->GetLength(),
      GetDC(),
      GetDC(),
      m_PrintRect,
      m_PageRect);

    m_PageBreaks.push_back(pos);
  }
}

void wxExPrintout::GetPageInfo(
  int* minPage, int* maxPage, int* pageFrom, int* pageTo)
{
  *minPage = 1;
  *maxPage = m_PageBreaks.size() - 1;
  *pageFrom = 1;
  *pageTo = m_PageBreaks.size() - 1;
}

void wxExPrintout::OnPreparePrinting()
{
  const double factor = 22.4;
  wxSize ppiScr;
  GetPPIScreen(&ppiScr.x, &ppiScr.y);

  wxPageSetupDialogData* dlg_data = wxExApp::GetPrinter()->GetPageSetupData();
  wxSize page = dlg_data->GetPaperSize();

  if (page.x == 0 || page.y == 0)
  {
    dlg_data->SetPaperSize(wxPAPER_A4);
    page = dlg_data->GetPaperSize();
  }

  page.x = (int)(page.x * ppiScr.x / factor);
  page.y = (int)(page.y * ppiScr.y / factor);

  m_PageRect = wxRect(0, 0, page.x, page.y);

  int left = (int)(dlg_data->GetMarginTopLeft().x * ppiScr.x / factor);
  int top = (int)(dlg_data->GetMarginTopLeft().y * ppiScr.y / factor);
  int right = (int)(dlg_data->GetMarginBottomRight().x * ppiScr.x / factor);
  int bottom = (int)(dlg_data->GetMarginBottomRight().y * ppiScr.y / factor);

  m_PrintRect = wxRect(
    left,
    top,
    page.x - (left + right),
    page.y - (top + bottom));

  CountPages();
}

bool wxExPrintout::OnPrintPage(int pageNum)
{
  wxASSERT(GetDC() != NULL);

  if (pageNum > (int)m_PageBreaks.size())
  {
    wxFAIL;
    return false;
  }

  SetScale(GetDC());

  m_Owner->FormatRange(
    true,
    m_PageBreaks[pageNum - 1],
    m_Owner->GetTextLength(),
    GetDC(),
    GetDC(),
    m_PrintRect,
    m_PageRect);

  wxFont font = *wxSMALL_FONT;
  font.SetStyle(wxNORMAL);
  font.SetWeight(wxBOLD);
  GetDC()->SetFont(font);
  GetDC()->SetTextForeground(*wxBLACK);
  GetDC()->SetTextBackground(*wxWHITE);
  GetDC()->SetPen(*wxBLACK_PEN);

  // Print a header.
  const wxString header = wxExPrintHeader(m_Owner->GetFileName());
  if (!header.empty())
  {
    GetDC()->DrawText(
      wxExTranslate(header, pageNum, m_PageBreaks.size()),
      m_PrintRect.GetTopLeft().x,
      m_PrintRect.GetTopLeft().y - 20);

    GetDC()->DrawLine(
      m_PrintRect.GetTopLeft().x,
      m_PrintRect.GetTopLeft().y - 10,
      m_PrintRect.GetBottomRight().x,
      m_PrintRect.GetTopLeft().y - 10);
  }

  // Print a footer
  const wxString footer = wxExPrintFooter();
  if (!footer.empty())
  {
    GetDC()->DrawText(
      wxExTranslate(footer, pageNum, m_PageBreaks.size() - 1),
      m_PrintRect.GetBottomRight().x / 2,
      m_PrintRect.GetBottomRight().y);

    GetDC()->DrawLine(
      m_PrintRect.GetTopLeft().x,
      m_PrintRect.GetBottomRight().y,
      m_PrintRect.GetBottomRight().x,
      m_PrintRect.GetBottomRight().y);
  }

  return true;
}

void wxExPrintout::SetScale(wxDC *dc)
{
  wxSize ppiScr, ppiPrt;
  GetPPIScreen(&ppiScr.x, &ppiScr.y);

  if (ppiScr.x == 0)
  {
    // Most possible gues  96 dpi.
    ppiScr.x = 96;
    ppiScr.y = 96;
  }

  GetPPIPrinter(&ppiPrt.x, &ppiPrt.y);

  if (ppiPrt.x == 0)
  {
    // Scaling factor to 1.
    ppiPrt.x = ppiScr.x;
    ppiPrt.y = ppiScr.y;
  }

  const wxSize dcSize = dc->GetSize();
  wxSize pageSize;
  GetPageSizePixels(&pageSize.x, &pageSize.y);

  const double factor = 0.8;
  const float scale_x = (float)(factor * ppiPrt.x * dcSize.x) / (float)(ppiScr.x * pageSize.x);
  const float scale_y = (float)(factor * ppiPrt.y * dcSize.y) / (float)(ppiScr.y * pageSize.y);

  dc->SetUserScale(scale_x, scale_y);
}
#endif // wxUSE_PRINTING_ARCHITECTURE

BEGIN_EVENT_TABLE(wxExSTC, wxStyledTextCtrl)
  EVT_IDLE(wxExSTC::OnIdle)
  EVT_KEY_DOWN(wxExSTC::OnKey)
  EVT_LEFT_UP(wxExSTC::OnMouse)
  EVT_RIGHT_UP(wxExSTC::OnMouse)
  EVT_MENU(wxID_DELETE, wxExSTC::OnCommand)
  EVT_MENU(wxID_JUMP_TO, wxExSTC::OnCommand)
  EVT_MENU(wxID_SAVE, wxExSTC::OnCommand)
  EVT_MENU(wxID_SELECTALL, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExSTC::OnCommand)
  EVT_MENU_RANGE(wxID_UNDO, wxID_REDO, wxExSTC::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_STC_LOWEST, ID_EDIT_STC_HIGHEST, wxExSTC::OnCommand)
  EVT_STC_DWELLEND(wxID_ANY, wxExSTC::OnStyledText)
//  EVT_STC_DWELLSTART(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MACRORECORD(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MARGINCLICK(wxID_ANY, wxExSTC::OnStyledText)
  EVT_STC_MODIFIED(wxID_ANY, wxExSTC::OnStyledText)
END_EVENT_TABLE()

wxExConfigDialog* wxExSTC::m_ConfigDialog = NULL;
vector <wxString> wxExSTC::m_Macro;
wxPathList wxExSTC::m_PathList;

#if wxUSE_PRINTING_ARCHITECTURE
wxPrinter* wxExSTC::m_Printer = NULL;
#endif

wxExSTC::wxExSTC(wxWindow* parent,
  const wxString& value,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxStyledTextCtrl(parent, id, pos, size, style, name)
  , m_FileSaveInMenu(false)
  , m_Flags(0)
  , m_GotoLineNumber(1)
  , m_MenuFlags(menu_flags)
  , m_PreviousLength(0)
{
  Initialize();

  if (!value.empty())
  {
    SetText(value);
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
  long style,
  const wxString& name)
  : wxStyledTextCtrl(parent, id, pos, size, style, name)
  , m_FileSaveInMenu(false)
  , m_Flags(0)
  , m_GotoLineNumber(1) // do not initialize with line_number, that might be 0 or -1
  , m_MenuFlags(menu_flags)
  , m_PreviousLength(0)
{
  Initialize();

  Open(filename, line_number, match, flags);
}

wxExSTC::wxExSTC(const wxExSTC& stc)
{
  wxStyledTextCtrl::Create(stc.GetParent());

  // Do not yet set GetFileName(), this is done by Open.
  // And m_Macro is shared, so not necessary.
  m_FileSaveInMenu = stc.m_FileSaveInMenu;
  m_Flags = stc.m_Flags;
  m_GotoLineNumber = stc.m_GotoLineNumber;
  m_PreviousLength = stc.m_PreviousLength;
  m_MenuFlags = stc.m_MenuFlags;

  Initialize();

  if (stc.GetFileName().IsOk())
  {
    Open(stc.GetFileName(), m_GotoLineNumber, wxEmptyString, m_Flags);
  }
}

void wxExSTC::AddAsciiTable()
{
  // Do not show an edge, eol or whitespace for ascii table.
  SetEdgeMode(wxSTC_EDGE_NONE);
  SetViewEOL(false);
  SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  // And override tab width.
  SetTabWidth(10);

  for (int i = 1; i <= 255; i++)
  {
    AddText(wxString::Format("%d\t%c", i, (wxChar)i));
    AddText((i % 5 == 0) ? GetEOL(): "\t");
  }

  EmptyUndoBuffer();
  SetSavePoint();
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

  PathListAdd(basepath);
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

void wxExSTC::AppendTextForced(const wxString& text, bool withTimestamp)
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

void wxExSTC::BuildPopupMenu(wxExMenu& menu)
{
  const wxString sel = GetSelectedText();
  const wxString link = GetTextAtCurrentPos();
  const int line_no = (!sel.empty() ? wxExGetLineNumberFromText(sel): GetLineNumberAtCurrentPos());

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
        menuSelection->AppendSeparator();
        menuSelection->Append(wxID_SORT_ASCENDING);
        menuSelection->Append(wxID_SORT_DESCENDING);
      }

      menu.AppendSeparator();
      menu.AppendSubMenu(menuSelection, _("&Selection"));
    }
    else
    {
      if (m_MenuFlags & STC_MENU_INSERT)
      {
        menu.AppendSeparator();
        menu.Append(ID_EDIT_INSERT_DATE, _("Insert Timestamp"));
        menu.Append(ID_EDIT_INSERT_SEQUENCE, wxExEllipsed(_("Insert Sequence")));
      }

      if (m_FileSaveInMenu && GetModify())
      {
        menu.AppendSeparator();
        menu.Append(wxID_SAVE);
      }
    }
  }

  if (sel.empty() && GetProperty("fold") == "1")
  {
    menu.AppendSeparator();
    menu.Append(ID_EDIT_TOGGLE_FOLD, _("&Toggle Fold\tCtrl+T"));
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

bool wxExSTC::CheckAutoComp(int key)
{
  static wxString autoc;

  if (isspace(GetCharAt(GetCurrentPos() - 1)))
  {
    autoc = (wxChar)key;
  }
  else
  {
    autoc += (wxChar)key;

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

bool wxExSTC::CheckSmartIndentation()
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
  if (line[i] == wxChar('\t') || line[i] == wxChar(' '))
  {
    InsertText(GetCurrentPos(), GetEOL());
    GotoLine(GetCurrentLine() + 1);

    while (line[i] == wxChar('\t') || line[i] == wxChar(' '))
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

void wxExSTC::CleanUp()
{
#if wxUSE_PRINTING_ARCHITECTURE
  delete m_Printer;
#endif
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
  SetKeyWords();
  SetGlobalStyles();
  SetMarkers();
  SetProperties();
  SetFolding();

  wxStringTokenizer tkz(
    GetFileName().GetLexer().GetColourings(),
    wxTextFile::GetEOL());

  while (tkz.HasMoreTokens())
  {
    SetStyle(tkz.GetNextToken());
  }

  // And finally colour the entire document.
  wxStyledTextCtrl::Colourise(0, GetLength() - 1);
}

// This is a static method, cannot use normal members here.
int wxExSTC::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  long flags,
  wxWindowID id)
{
  vector<wxExConfigItem> items;

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
    _("Tab width"), 1, (int)wxConfigBase::Get()->ReadLong(_("Edge Column"), 80), page));
  items.push_back(wxExConfigItem(
    _("Indent"), 1, (int)wxConfigBase::Get()->ReadLong(_("Edge Column"), 80), page));

  set<wxString> bchoices;
  bchoices.insert(_("End of line"));
  bchoices.insert(_("Line numbers"));
  bchoices.insert(_("Use tabs"));
  items.push_back(wxExConfigItem(bchoices, page));

  map<long, const wxString> choices;
  choices.insert(make_pair(wxSTC_WS_INVISIBLE, _("Invisible")));
  choices.insert(make_pair(wxSTC_WS_VISIBLEAFTERINDENT, _("Invisible after ident")));
  choices.insert(make_pair(wxSTC_WS_VISIBLEALWAYS, _("Visible always")));
  items.push_back(wxExConfigItem(_("WhiteSpace"), choices, true, page));

  map<long, const wxString> wchoices;
  wchoices.insert(make_pair(wxSTC_WRAP_NONE, _("None")));
  wchoices.insert(make_pair(wxSTC_WRAP_WORD, _("Word")));
  wchoices.insert(make_pair(wxSTC_WRAP_CHAR, _("Char")));
  items.push_back(wxExConfigItem(_("Wrap line"), wchoices, true, page));

  map<long, const wxString> vchoices;
  vchoices.insert(make_pair(wxSTC_WRAPVISUALFLAG_NONE, _("None")));
  vchoices.insert(make_pair(wxSTC_WRAPVISUALFLAG_END, _("End")));
  vchoices.insert(make_pair(wxSTC_WRAPVISUALFLAG_START, _("Start")));
  items.push_back(wxExConfigItem(_("Wrap visual flags"), vchoices, true, page));

  if (!(flags & STC_CONFIG_SIMPLE))
  {
    items.push_back(wxExConfigItem(_("Edge column"), 0, 500, _("Edge")));

    map<long, const wxString> echoices;
    echoices.insert(make_pair(wxSTC_EDGE_NONE, _("None")));
    echoices.insert(make_pair(wxSTC_EDGE_LINE, _("Line")));
    echoices.insert(make_pair(wxSTC_EDGE_BACKGROUND, _("Background")));
    items.push_back(wxExConfigItem(_("Edge line"), echoices, true, _("Edge")));

    items.push_back(wxExConfigItem(_("Auto fold"), CONFIG_INT, _("Folding")));
    items.push_back(wxExConfigItem()); // spacer
    items.push_back(wxExConfigItem(_("Indentation guide"), CONFIG_CHECKBOX, _("Folding")));

    map<long, const wxString> fchoices;
    fchoices.insert(make_pair(wxSTC_FOLDFLAG_BOX, _("Box")));
    fchoices.insert(make_pair(wxSTC_FOLDFLAG_LINEBEFORE_EXPANDED, _("Line before expanded")));
    fchoices.insert(make_pair(wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED, _("Line before contracted")));
    fchoices.insert(make_pair(wxSTC_FOLDFLAG_LINEAFTER_EXPANDED, _("Line after expanded")));
    fchoices.insert(make_pair(wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED, _("Line after contracted")));
    fchoices.insert(make_pair(wxSTC_FOLDFLAG_LEVELNUMBERS, _("Level numbers")));
    items.push_back(wxExConfigItem(_("Fold flags"), fchoices, false, _("Folding")));

    items.push_back(wxExConfigItem(_("CallTip"), CONFIG_COLOUR, _("Colour")));

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
  if (!wxConfigBase::Get()->Exists(_("CallTip")))
  {
    wxConfigBase::Get()->SetRecordDefaults(true);
  }

  CallTipSetBackground(wxConfigBase::Get()->ReadObject(
    _("CallTip"), wxColour("YELLOW")));

  SetEdgeColumn(wxConfigBase::Get()->ReadLong(_("Edge column"), 80)); // see also lexer
  SetEdgeMode(wxConfigBase::Get()->ReadLong(_("Edge line"), wxSTC_EDGE_NONE) == wxSTC_EDGE_LINE);
  SetFoldFlags(wxConfigBase::Get()->ReadLong( _("Fold flags"),
    wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));
  SetIndent(wxConfigBase::Get()->ReadLong(_("Indent"), 4));
  SetIndentationGuides(wxConfigBase::Get()->ReadBool(_("Indentation guide"), false));

  SetMarginWidth(m_MarginDividerNumber, wxConfigBase::Get()->ReadLong(_("Divider"), 16));
  SetMarginWidth(m_MarginFoldingNumber, wxConfigBase::Get()->ReadLong(_("Folding"), 16));
  SetMarginWidth(m_MarginLineNumber,
    (wxConfigBase::Get()->ReadBool(_("Line numbers"), false) ?
      wxConfigBase::Get()->ReadLong(_("Line number"), TextWidth(m_MarginLineNumber, "999999")): 0));

  SetTabWidth(wxConfigBase::Get()->ReadLong(_("Tab width"), 4));
  SetUseTabs(wxConfigBase::Get()->ReadBool(_("Use tabs"), false));
  SetViewEOL(wxConfigBase::Get()->ReadBool(_("End of line"), false));
  SetViewWhiteSpace(wxConfigBase::Get()->ReadLong(_("WhiteSpace"), wxSTC_WS_INVISIBLE));
  SetWrapMode(wxConfigBase::Get()->ReadLong(_("Wrap line"), wxSTC_WRAP_NONE));
  SetWrapVisualFlags(wxConfigBase::Get()->ReadLong(_("Wrap visual flags"), wxSTC_WRAPVISUALFLAG_END));

  if (wxConfigBase::Get()->IsRecordingDefaults())
  {
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
      const char value = GetSelectedText().GetChar(0);
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
      ReplaceSelection(wxString::Format("%c", (wxChar)new_value));
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

void wxExSTC::DoFileLoad(bool synced)
{
  wxBusyCursor wait;

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
    SetMarkers();
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
    wxExApp::Log(msg);
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(msg);
#endif
    PropertiesMessage();
  }
  else
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(GetFileName(), STAT_SYNC);
    UpdateStatusBar("PaneLines");
#endif
  }

  if (GetFileName() == wxExLogfileName())
  {
    DocumentEnd();
  }
}

void wxExSTC::DoFileSave()
{
  const wxCharBuffer& buffer = GetTextRaw(); 
  Write(buffer.data(), buffer.length());

  const wxString msg = _("Saved") + ": " + GetFileName().GetFullPath();
  wxExApp::Log(msg);
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

bool wxExSTC::FindNext(const wxString& text, bool find_next)
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
  SetSearchFlags(FindReplaceDataFlags());

  if (SearchInTarget(text) < 0)
  {
    wxExFindResult(text, find_next, recursive);
    
    if (!recursive)
    {
      recursive = true;
      FindNext(text, find_next);
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

int wxExSTC::FindReplaceDataFlags() const
{
  const wxExFindReplaceData* frd = wxExApp::GetFindReplaceData();

  int flags = 0;

  if (frd->IsRegularExpression())  flags |= wxSTC_FIND_REGEXP;
  if (frd->MatchWord()) flags |= wxSTC_FIND_WHOLEWORD;
  if (frd->MatchCase()) flags |= wxSTC_FIND_MATCHCASE;

  return flags;
}

void wxExSTC::FoldAll()
{
  if (GetProperty("fold") != "1") return;

  wxBusyCursor wait;

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

int wxExSTC::GetLineNumberAtCurrentPos()
{
  // This method is used by LinkOpen.
  // So, if no line number present return 0, otherwise link open jumps to last line.
  const int pos = GetCurrentPos();
  const int line_no = LineFromPosition(pos);

  // Cannot use GetLine, as that includes EOF, and then the ToLong does not
  // return correct number.
  const wxString text = GetTextRange(PositionFromLine(line_no), GetLineEndPosition(line_no));

  return wxExGetLineNumberFromText(text);
}

const wxString wxExSTC::GetSearchText()
{
  const wxString selection = GetSelectedText();
  if (!selection.empty() && wxExGetNumberOfLines(selection) == 1)
    wxExApp::GetFindReplaceData()->SetFindString(selection);
  return wxExApp::GetFindReplaceData()->GetFindString();
}

const wxString wxExSTC::GetTextAtCurrentPos()
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

const wxString wxExSTC::GetWordAtPos(int pos)
{
  const int word_start = WordStartPosition(pos, true);
  const int word_end = WordEndPosition(pos, true);

  if (word_start == word_end && word_start < GetTextLength())
  {
    const wxString word = GetTextRange(word_start, word_start + 1);
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
    const wxString word = GetTextRange(word_start, word_end);
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

  GotoLineAndSelect(val, wxEmptyString);

  return true;
}

void wxExSTC::GotoLineAndSelect(int line_number, const wxString& text)
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

  // TODO: This is not unicode compatible.
  const unsigned char c = word.GetChar(0);

  if (c < 32 || c > 125)
  {
    CallTipShow(pos, wxString::Format("hex: %x dec: %d", c, c));
    return;
  }

  long base10_val, base16_val;
  const bool base10_ok = word.ToLong(&base10_val);
  const bool base16_ok = word.ToLong(&base16_val, 16);

  if (base10_ok || base16_ok)
  {
    wxString text;

    if      ( base10_ok && !base16_ok) text = wxString::Format("hex: %lx", base10_val);
    else if (!base10_ok &&  base16_ok) text = wxString::Format("dec: %ld", base16_val);
    else if ( base10_ok &&  base16_ok) text = wxString::Format("hex: %lx dec: %ld", base10_val, base16_val);

    CallTipShow(pos, text);
  }
  else
  {
    // If this word can be found locally, show the data type.
    // This was still present in v1.3.
  }
}

void wxExSTC::Initialize()
{
  m_MacroIsRecording = false;
  m_MarginDividerNumber = 1;
  m_MarginFoldingNumber = 2;
  m_MarginLineNumber = 0;

#if wxUSE_PRINTING_ARCHITECTURE
  if (m_Printer == NULL)
    m_Printer = new wxPrinter;
#endif

  UsePopUp(false);
#ifdef __WXMSW__
  EOLModeUpdate(wxSTC_EOL_CRLF);
#else
  EOLModeUpdate(wxSTC_EOL_LF);
#endif
  ConfigGet();

  SetMarginType(m_MarginLineNumber, wxSTC_MARGIN_NUMBER);
  SetMarginType(m_MarginDividerNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginType(m_MarginFoldingNumber, wxSTC_MARGIN_SYMBOL);
  SetMarginMask(m_MarginFoldingNumber, wxSTC_MASK_FOLDERS);
  SetMarginSensitive(m_MarginFoldingNumber, true);

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
  wxExLexer lexer = GetFileName().GetLexer();

  if (wxExApp::GetLexers()->ShowDialog(this, lexer, caption))
  {
    SetLexer(lexer.GetScintillaLexer(), true); // forced
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
      // README: Code added since wxWidgets 2.7.0, as that normalized the file.
      //fullpath = wxConfigBase::Get()->GetPathList().FindAbsoluteValidPath(link);
      wxString strend = link;

      for (size_t i=0; i < m_PathList.GetCount() && fullpath.empty(); i++)
      {
        wxString strstart = m_PathList.Item(i);
        if (!strstart.IsEmpty() && strstart.Last() != wxFileName::GetPathSeparator())
            strstart += wxFileName::GetPathSeparator();

        if (wxFileExists(strstart + strend))
            fullpath = strstart + strend;
      }
    }
  }

  if (!fullpath.empty() && open_link)
  {
    return Open(fullpath, line_number, wxEmptyString, m_Flags | STC_OPEN_FROM_LINK);
  }

  filename = wxFileName(fullpath).GetFullName();
  
  return !fullpath.empty();
}

void wxExSTC::MacroPlayback()
{
  wxASSERT(MacroIsRecorded());

  for (
    vector<wxString>::const_iterator it = m_Macro.begin();
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

void wxExSTC::MatchHexBrace()
{
    const int col = GetColumn(GetCurrentPos());
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

      BraceHighlight(GetCurrentPos(),
        PositionFromLine(GetCurrentLine()) + start_hex_field + each_hex_field * offset + space);
    }
    else if (col >= start_hex_field)
    {
      if (GetCharAt(GetCurrentPos()) != ' ')
      {
        int space = 0;

        if (col >= start_hex_field + space_between_fields + (bytes_per_line * each_hex_field) / 2)
        {
          space++;
        }

        const int offset = (col - (start_hex_field + space)) / each_hex_field;

        BraceHighlight(GetCurrentPos(),
          PositionFromLine(GetCurrentLine()) + start_ascii_field + offset);
      }
    }
    else
    {
      BraceHighlight(wxSTC_INVALID_POSITION, wxSTC_INVALID_POSITION);
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
    {
      wxBusyCursor wait;
      for (int i = 0; i < GetLineCount(); i++) EnsureVisible(i);
    }
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
    if (!sel.empty())
    {
      wxString filename;
      LinkOpen(sel, filename, wxExGetLineNumberFromText(sel));
    }
    else
    {
      wxString filename;
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

  case ID_EDIT_INSERT_DATE: AddText(wxDateTime::Now().Format()); break;
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
    !IsOpened() &&
    // the readonly flags bit of course can differ from file actual readonly mode,
    // therefore add this check
    !(m_Flags & STC_OPEN_READ_ONLY) &&
    GetFileName().GetStat().IsOk() &&
    GetFileName().GetStat().IsReadOnly() != GetReadOnly())
  {
    FileReadOnlyAttributeChanged();
  }
}

void wxExSTC::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

  if (GetReadOnly() && isascii(key))
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(_("Document is readonly"));
#endif
    return;
  }

  if (m_Flags & STC_OPEN_HEX)
  {
    event.Skip();
    MatchHexBrace();
    return;
  }

  if (key == WXK_RETURN)
  {
    if (AutoCompActive())
    {
      event.Skip();
    }
    else if (!CheckSmartIndentation())
    {
      event.Skip();
    }
  }
  else if (key == WXK_LEFT || key == WXK_RIGHT)
  {
    event.Skip();

    if (key == WXK_LEFT)
    {
      if (!CheckBrace(GetCurrentPos() - 1))
        CheckBrace(GetCurrentPos() - 2);
    }
    else if (!CheckBrace(GetCurrentPos()))
    {
      CheckBrace(GetCurrentPos() + 1);
    }
  }
  else
  {
    event.Skip();
    CheckAutoComp(key);
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

void wxExSTC::PathListAdd(const wxString& path)
{
  if (!wxFileName::DirExists(path))
  {
    wxLogError("Path: %s does not exist", path.c_str());
    return;
  }

  if (m_PathList.Index(path) == wxNOT_FOUND)
  {
    // README: Changed normal Add into this one for wxWidgets 2.7.
    // It seems the normal Add normalizes the path, we do not want that.
    ((wxArrayString *)&m_PathList)->Add(path);
  }
}

void wxExSTC::PathListInit()
{
  m_PathList.Clear();

  wxStringTokenizer tkz(
    wxConfigBase::Get()->Read(_("Include directory")),
    ";");

  while (tkz.HasMoreTokens())
  {
    const wxString includedir = tkz.GetNextToken();
    m_PathList.Add(includedir);
  }
}

#if wxUSE_PRINTING_ARCHITECTURE
void wxExSTC::Print(bool prompt)
{
  wxPrintData* data = wxExApp::GetPrinter()->GetPrintData();
  m_Printer->GetPrintDialogData().SetPrintData(*data);
  m_Printer->Print(this, new wxExPrintout(this), prompt);
}
#endif

#if wxUSE_PRINTING_ARCHITECTURE
void wxExSTC::PrintPreview()
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
    wxExPrintCaption(GetFileName()));

  frame->Initialize();
  frame->Show();
}
#endif

void wxExSTC::PropertiesMessage()
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
    SetControlCharSymbol(0);

    const int message = (get_only_new_data ? SCI_APPENDTEXT: SCI_ADDTEXT);

    // README: The stc.h equivalents AddText, AddTextRaw, InsertText, InsertTextRaw do not add the length.
    // So for binary files this is the only way for opening.
    SendMsg(message, buffer.length(), (wxIntPtr)(const char *)buffer.data());
  }
  else
  {
    SetControlCharSymbol('x');

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

void wxExSTC::ResetMargins(bool divider_margin)
{
  SetMarginWidth(m_MarginFoldingNumber, 0);
  SetMarginWidth(m_MarginLineNumber, 0);

  if (divider_margin)
  {
    SetMarginWidth(m_MarginDividerNumber, 0);
  }
}

void wxExSTC::Replace(
  const wxString& find_text, 
  const wxString& replace_text,
  bool is_regular_expression,
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

  if (is_regular_expression)
    ReplaceTargetRE(replace_text);
  else
    ReplaceTarget(replace_text);

  FindNext(find_text, find_next);
}
  
void wxExSTC::ReplaceAll(
  const wxString& find_text,
  const wxString& replace_text,
  bool is_regular_expression)
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

  while (SearchInTarget(find_text) != -1)
  {
    if (GetTargetStart() == GetTargetEnd())
    {
      wxFAIL_MSG("Target start and end are equal");
      break;
    }

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
      if (is_regular_expression)
        length = ReplaceTargetRE(replace_text);
      else
        length = ReplaceTarget(replace_text);

      nr_replacements++;
    }

    if (length == -1)
    {
      break;
    }

    if (target_start + length >= GetLength() - 1) break;

    SetTargetStart(target_start + length);
    SetTargetEnd(GetLength() - selection_from_end);
  }

  EndUndoAction();

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(wxString::Format(_("Replaced: %d occurrences of: %s"),
    nr_replacements, find_text.c_str()));
#endif
}

void wxExSTC::SequenceDialog()
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

void wxExSTC::SetFolding()
{
  if (GetProperty("fold") == "1")
  {
    SetMarginWidth(m_MarginFoldingNumber, wxConfigBase::Get()->ReadLong(_("Folding"), 16));

    SetFoldFlags(
      wxConfigBase::Get()->ReadLong(_("Fold Flags"),
      wxSTC_FOLDFLAG_LINEBEFORE_CONTRACTED | wxSTC_FOLDFLAG_LINEAFTER_CONTRACTED));

    // C Header files usually contain #ifdef statements.
    // Folding them might fold the entire file. Therefore don't fold preprocessor.
    if (GetFileName().GetLexer().GetScintillaLexer() == "cpp" &&
        GetFileName().GetExt() == "h")
    {
      SetProperty("fold.preprocessor", "0");
    }
  }
  else
  {
    SetMarginWidth(m_MarginFoldingNumber, 0);
  }
}

void wxExSTC::SetGlobalStyles()
{
  StyleClearAll();

  if (!(m_Flags & STC_OPEN_HEX))
  {
    for (
      vector<wxString>::const_iterator it = wxExApp::GetLexers()->GetStyles().begin();
      it != wxExApp::GetLexers()->GetStyles().end();
      ++it)
    {
      SetStyle(*it);
    }

    for (
      map<int, int>::const_iterator ind = wxExApp::GetLexers()->GetIndicators().begin();
      ind != wxExApp::GetLexers()->GetIndicators().end();
      ++ind)
    {
      IndicatorSetStyle(ind->first, ind->second);
    }
  }
  else
  {
    for (
      vector<wxString>::const_iterator it = wxExApp::GetLexers()->GetStylesHex().begin();
      it != wxExApp::GetLexers()->GetStylesHex().end();
      ++it)
    {
      SetStyle(*it);
    }
  }
}

void wxExSTC::SetKeyWords()
{
  // Reset keywords, also if no lexer is available.
  for (size_t setno = 0; setno < wxSTC_KEYWORDSET_MAX; setno++)
  {
    wxStyledTextCtrl::SetKeyWords(setno, wxEmptyString);
  }

  // Readme: The Scintilla lexer only recognized lower case words, apparently.
  for (
    std::map< int, std::set<wxString> >::const_iterator it = GetFileName().GetLexer().GetKeywordsSet().begin();
    it != GetFileName().GetLexer().GetKeywordsSet().end();
    ++it)
  {
    wxStyledTextCtrl::SetKeyWords(
      it->first,
      GetFileName().GetLexer().GetKeywordsString(it->first).Lower());
  }
}

void wxExSTC::SetLexer(const wxString& lexer, bool forced)
{
  if (forced)
  {
    ClearDocumentStyle();

    // Reset all old properties. 
    // Should be before GetFileName().SetLexer().
    wxStringTokenizer properties(
      GetFileName().GetLexer().GetProperties(),
      wxTextFile::GetEOL());

    while (properties.HasMoreTokens())
    {
      wxStringTokenizer property(properties.GetNextToken(), "=");

      // Don't put key, value into SetProperty, as that might parse value first,
      // reversing the two.
      const wxString key = property.GetNextToken();
      const wxString value = property.GetNextToken(); // ignore

      SetProperty(key, wxEmptyString);
    }

    SetFileNameLexer(lexer, "forced");
  }
  else
  {
    SetFileNameLexer(lexer, GetLine(0));
  }

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

void wxExSTC::SetMarkers()
{
  for (
    vector<wxExMarker>::const_iterator it = wxExApp::GetLexers()->GetMarkers().begin();
    it != wxExApp::GetLexers()->GetMarkers().end();
    ++it)
  {
     MarkerDefine(
       it->GetMarkerNumber(),
       it->GetMarkerSymbol(),
       it->GetForegroundColour(),
       it->GetBackgroundColour());
  }
}

void wxExSTC::SetProperties()
{
  wxStringTokenizer properties(
    GetFileName().GetLexer().GetProperties(),
    wxTextFile::GetEOL());

  while (properties.HasMoreTokens())
  {
    wxStringTokenizer property(properties.GetNextToken(), "=");

    // Don't put key, value into SetProperty, as that might parse value first,
    // reversing the two.
    const wxString key = property.GetNextToken();
    const wxString value = property.GetNextToken();

    SetProperty(key, value);
  }
}

void wxExSTC::SetStyle(const wxString& style)
{
  // E.g.
  // 1,2,3=fore:light steel blue,italic,size:8
  // 1,2,3 are the scintilla_styles, and the rest is spec

  wxStringTokenizer stylespec(style, "=");

  wxStringTokenizer scintilla_styles(stylespec.GetNextToken(), ",");
  const wxString spec = stylespec.GetNextToken();

  // So for each scintilla style set the spec.
  while (scintilla_styles.HasMoreTokens())
  {
    StyleSetSpec(atoi(scintilla_styles.GetNextToken().c_str()), spec);
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
  multimap<wxString, wxString> mm;
  while (tkz.HasMoreTokens())
  {
    const wxString line = tkz.GetNextToken() + GetEOL();

    // Use an empty key if line is to short.
    wxString key;

    if (val - 1 < (long)line.length())
    {
      key = line.substr(val - 1);
    }

    mm.insert(make_pair(key, line));
  }

  // The multimap is already sorted, just iterate to get all lines back.
  wxString text;
  if (sort_ascending)
  {
    for (
      multimap<wxString, wxString>::const_iterator it = mm.begin();
      it != mm.end();
      ++it)
    {
      text += it->second;
    }
  }
  else
  {
    for (
      multimap<wxString, wxString>::reverse_iterator it = mm.rbegin();
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

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Macro recording"));
#endif

  wxStyledTextCtrl::StartRecord();
}

void wxExSTC::StopRecord()
{
  wxASSERT(m_MacroIsRecording);

  m_MacroIsRecording = false;

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Macro is recorded"));
#endif

  wxStyledTextCtrl::StopRecord();
}

#if wxUSE_STATUSBAR
void wxExSTC::UpdateStatusBar(const wxString& pane)
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
      GetSelection(&start, &end);
      const int len  = end - start;
      const int line = GetCurrentLine() + 1;
      const int pos = GetCurrentPos() + 1 - PositionFromLine(line - 1);

      if (len == 0) text = wxString::Format("%d,%d", line, pos);
      else
      {
        // There might be NULL's inside selection.
        // So use the GetSelectedTextRaw variant.
        const int number_of_lines = wxExGetNumberOfLines(GetSelectedTextRaw());
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

wxExSTCEntryDialog::wxExSTCEntryDialog(wxWindow* parent,
  const wxString& caption,
  const wxString& text,
  const wxString& prompt,
  long button_style,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxExDialog(parent, caption, button_style, id, pos, size, style, name)
{
  if (!prompt.empty())
  {
    AddUserSizer(CreateTextSizer(prompt), wxSizerFlags().Center());
  }

  // Currently we cannot find in the component,
  // so no wxExSTC::STC_MENU_FIND flag,
  // as this requires the component to be visible to public interface,
  // and the applications to forward the find events.
  m_STC = new wxExSTC(
    this, 
    text, 
    wxExSTC::STC_MENU_SIMPLE,
    wxID_ANY, 
    pos, 
    size);

  // Override defaults from config.
  m_STC->SetEdgeMode(wxSTC_EDGE_NONE);
  m_STC->ResetMargins();
  m_STC->SetViewEOL(false);
  m_STC->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);

  if ((button_style & wxCANCEL) == 0 &&
      (button_style & wxNO) == 0)
  {
    // You did not specify one of these buttons,
    // so you cannot cancel the operation.
    // Therefore make the component readonly.
    m_STC->SetReadOnly(true);
  }

  AddUserSizer(m_STC);

  LayoutSizers();
}

void wxExSTCEntryDialog::SetText(const wxString& text)
{
  const bool readonly = m_STC->GetReadOnly();

  if (readonly)
  {
    m_STC->SetReadOnly(false);
  }

  m_STC->SetText(text);

  if (readonly)
  {
    m_STC->SetReadOnly(true);
  }
}

#endif // wxUSE_GUI
