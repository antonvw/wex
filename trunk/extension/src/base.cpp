/******************************************************************************\
* File:          base.cpp
* Purpose:       Implementation of wxWidgets base extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/tooltip.h>
#include <wx/extension/extension.h>
#include <wx/extension/listview.h>
#include <wx/extension/stc.h>
#include <wx/extension/tool.h>

using namespace std;

#if wxUSE_GUI
exDialog::exDialog(wxWindow* parent,
  const wxString& title,
  long flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxDialog(parent, id, title, pos, size, style, name)
  , m_Flags(flags)
  , m_TopSizer(new wxFlexGridSizer(1, 0, 0))
  , m_UserSizer(new wxFlexGridSizer(1, 0, 0))
{
}

wxSizerItem* exDialog::AddUserSizer(
  wxWindow* window,
  const wxSizerFlags& flags)
{
  wxSizerItem* item = m_UserSizer->Add(window, flags);

  if (flags.GetFlags() & wxEXPAND)
  {
    m_UserSizer->AddGrowableRow(m_UserSizer->GetChildren().GetCount() - 1);
  }

  return item;
}

wxSizerItem* exDialog::AddUserSizer(
  wxSizer* sizer,
  const wxSizerFlags& flags)
{
  wxSizerItem* item = m_UserSizer->Add(sizer, flags);

  if (flags.GetFlags() & wxEXPAND)
  {
    m_UserSizer->AddGrowableRow(m_UserSizer->GetChildren().GetCount() - 1);
  }

  return item;
}


void exDialog::BuildSizers()
{
  m_TopSizer->AddGrowableCol(0);
  m_UserSizer->AddGrowableCol(0);

  wxSizerFlags flag;
  flag.Expand().Center().Border();

  // The top sizer starts with a spacer, for a nice border.
  m_TopSizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  // Then place the growable user sizer.
  m_TopSizer->Add(m_UserSizer, flag);
  m_TopSizer->AddGrowableRow(m_TopSizer->GetChildren().GetCount() - 1); // so this is the user sizer

  // Then the button sizer.
  wxSizer* sbz = CreateSeparatedButtonSizer(m_Flags);

  if (sbz != NULL)
  {
    m_TopSizer->Add(sbz, flag);
  }

  // The top sizer ends with a spacer as well.
  m_TopSizer->AddSpacer(wxSizerFlags::GetDefaultBorder());

  SetSizerAndFit(m_TopSizer);
}

#if wxUSE_STATUSBAR
exStatusBar* exFrame::m_StatusBar = NULL;
map<wxString, exPane> exFrame::m_Panes;
#endif

BEGIN_EVENT_TABLE(exFrame, wxFrame)
  EVT_CLOSE(exFrame::OnClose)
#if wxUSE_STATUSBAR
  EVT_UPDATE_UI(ID_EDIT_STATUS_BAR, exFrame::OnUpdateUI)
#endif
END_EVENT_TABLE()

exFrame::exFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style,
  const wxString& name)
  : wxFrame(parent, id, title, wxDefaultPosition, wxDefaultSize, style, name)
  , m_KeepPosAndSize(true)
{
  if (exApp::GetConfig("Frame/Maximized", 0l))
  {
    Maximize(true);
  }

  SetSize(
    exApp::GetConfig("Frame/X", 100),
    exApp::GetConfig("Frame/Y", 100),
    exApp::GetConfig("Frame/Width", 450),
    exApp::GetConfig("Frame/Height", 350));
}

exFrame::exFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxFrame(parent, id, title, pos, size, style, name)
  , m_KeepPosAndSize(false)
{
}

exFrame::~exFrame()
{
#if wxUSE_STATUSBAR
  delete m_StatusBar;
#endif
}

exListView* exFrame::GetFocusedListView()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, exListView);
}

exSTC* exFrame::GetFocusedSTC()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, exSTC);
}

#if wxUSE_STATUSBAR
const exPane exFrame::GetPane(int pane) const
{
  for (
    map<wxString, exPane>::const_iterator it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.m_No == pane)
    {
      return it->second;
    }
  }

  return exPane();
}

int exFrame::GetPaneField(const wxString& pane)
{
  if (!m_Panes.empty())
  {
    map<wxString, exPane>::const_iterator it = m_Panes.find(pane);

    if (it != m_Panes.end())
    {
      return it->second.m_No;
    }
  }

  return -1;
}
#endif // wxUSE_STATUSBAR

void exFrame::OnClose(wxCloseEvent& event)
{
  if (m_KeepPosAndSize)
  {
    // Set config values that might have changed.
    if (IsMaximized())
    {
      exApp::SetConfig("Frame/Maximized", 1);
    }
    else
    {
      exApp::SetConfig("Frame/Maximized", 0);
      const wxRect rect = GetRect();
      exApp::SetConfig("Frame/X", rect.GetX());
      exApp::SetConfig("Frame/Y", rect.GetY());
      exApp::SetConfig("Frame/Width", rect.GetWidth());
      exApp::SetConfig("Frame/Height", rect.GetHeight());
    }
  }

  event.Skip();
}

#if wxUSE_STATUSBAR
wxStatusBar* exFrame::OnCreateStatusBar(
  int number,
  long style,
  wxWindowID id,
  const wxString& name)
{
  m_StatusBar = new exStatusBar(this, id, style, name);
  m_StatusBar->SetFieldsCount(number);
  return m_StatusBar;
}
#endif

wxToolBar* exFrame::OnCreateToolBar(
  long style, 
  wxWindowID id, 
  const wxString& name)
{
  m_ToolBar = new exToolBar(this, 
    id,
    wxDefaultPosition, 
    wxDefaultSize, 
    style,
    wxSize(16, 15),
    name);

  return m_ToolBar;
}

void exFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  exSTC* stc = GetFocusedSTC();
  if (stc == NULL) return;

  switch (event.GetId())
  {
#if wxUSE_STATUSBAR
  case ID_EDIT_STATUS_BAR: stc->UpdateStatusBar("PaneLines"); break;
#endif
  }
}

bool exFrame::OpenFile(
  const exFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  exSTC* stc = GetFocusedSTC();

  if (stc != NULL)
  {
    // Remove link flags;
    const long new_flags = 0;
    return stc->Open(filename, line_number, match, new_flags);
  }

  return false;
}

#if wxUSE_STATUSBAR
void exFrame::SetupStatusBar(
  const vector<exPane>& panes,
  long style,
  wxWindowID id,
  const wxString& name)
{
  wxFrame::CreateStatusBar(panes.size(), style, id, name);

  int* styles = new int[panes.size()];
  int* widths = new int[panes.size()];

  for (
    vector<exPane>::const_iterator it = panes.begin();
    it != panes.end();
    ++it)
  {
    m_Panes[it->m_Name] = *it;
    styles[it->m_No] = it->m_Style;
    widths[it->m_No] = it->m_Width;
  }

  m_StatusBar->SetStatusStyles(panes.size(), styles);
  m_StatusBar->SetStatusWidths(panes.size(), widths);

  delete[] styles;
  delete[] widths;
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
void exFrame::StatusBarDoubleClicked(int field, const wxPoint& point)
{
  if (field == GetPaneField("PaneLines"))
  {
    exSTC* stc = GetSTC();
    if (stc != NULL) stc->GotoDialog();
  }
  else if (field == GetPaneField("PaneLexer"))
  {
    exSTC* stc = GetSTC();
    if (stc != NULL) stc->LexerDialog();
  }
  else if (field == GetPaneField("PaneFileType"))
  {
    exSTC* stc = GetSTC();
    if (stc != NULL) stc->FileTypeMenu();
  }
  else if (field == GetPaneField("PaneItems"))
  {
    exListView* list = GetListView();
    if (list != NULL) list->GotoDialog();
  }
}

// This is a static method, so you cannot call wxFrame::SetStatusText.
void exFrame::StatusText(const wxString& text, const wxString& pane)
{
  if (m_StatusBar == NULL || m_Panes.empty())
  {
    // You did not ask for a status bar, so ignore all.
    return;
  }

  const int field = GetPaneField(pane);

  if (field >= 0)
  {
    // Especially with statusbar updating (in the OnIdle for exSTC or your application), most
    // of the time the statusbar does not change.
    // To avoid flicker, therefore only set if something changes.
    if (m_StatusBar->GetStatusText(field) != text)
    {
      m_StatusBar->SetStatusText(text, field);
    }
  }
}
#endif // wxUSE_STATUSBAR

exInterface::exInterface()
{
  m_FindReplaceDialog = NULL;
}

exInterface::~exInterface()
{
  if (m_FindReplaceDialog != NULL)
  {
    wxDELETE(m_FindReplaceDialog);
  }
}

void exInterface::FindDialog(wxWindow* parent, const wxString& caption)
{
  if (m_FindReplaceDialog != NULL)
  {
    wxDELETE(m_FindReplaceDialog);
  }

  m_FindReplaceDialog = new wxFindReplaceDialog(
    parent,
    exApp::GetConfig()->GetFindReplaceData(),
    caption);

  m_FindReplaceDialog->Show();
}

bool exInterface::FindNext(const wxString& text, bool find_next)
{
  return false;
}

bool exInterface::FindResult(const wxString& text, bool find_next, bool& recursive)
{
  if (!recursive)
  {
    recursive = true;
    const wxString where = (find_next) ? _("bottom"): _("top");
    exFrame::StatusText(
      _("Searching for") + " '" + exSkipWhiteSpace(text) + "' " + _("hit") + " " + where);
    return FindNext(text, find_next);
  }
  else
  {
    recursive = false;
    wxBell();
    // Same text also displayed in exSTC.
    exFrame::StatusText("'" + exSkipWhiteSpace(text) + "' " + _("not found"));
    return false;
  }
}

void exInterface::OnFindDialog(wxFindDialogEvent& event)
{
  exFindReplaceData* frd = exApp::GetConfig()->GetFindReplaceData();

  const bool find_next = (frd->GetFlags() & wxFR_DOWN);

  if (event.GetEventType() == wxEVT_COMMAND_FIND_CLOSE)
  {
    wxDELETE(m_FindReplaceDialog);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_FIND)
  {
    FindNext(frd->GetFindString(), find_next);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_FIND_NEXT)
  {
    FindNext(frd->GetFindString(), find_next);
  }
  else
  {
    wxLogError(FILE_INFO("Unhandled"));
  }
}

void exInterface::Print()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  exApp::GetPrinter()->PrintText(BuildPage());
#endif
}

void exInterface::PrintPreview()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  exApp::GetPrinter()->PreviewText(BuildPage());
#endif
}

void exInterface::ReplaceDialog(wxWindow* parent, const wxString& caption)
{
  if (m_FindReplaceDialog != NULL)
  {
    wxDELETE(m_FindReplaceDialog);
  }

  m_FindReplaceDialog = new wxFindReplaceDialog(
    parent,
    exApp::GetConfig()->GetFindReplaceData(),
    caption,
    wxFR_REPLACEDIALOG);

  m_FindReplaceDialog->Show();
}

exManagedFrame::exManagedFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style,
  const wxString& name)
  : exFrame(parent, id, title, style, name)
{
  m_Manager.SetManagedWindow(this);
}

exManagedFrame::~exManagedFrame()
{
  m_Manager.UnInit();
}

void exManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  if (!info.IsOk())
  {
    wxLogError("Invalid pane: " + pane);
    return;
  }

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}

exMenu::exMenu(long style)
  : m_Style(style)
{
}

exMenu::exMenu(const exMenu& menu)
  : m_Style(menu.m_Style)
{
}

wxMenuItem* exMenu::Append(
  int id,
  const wxString& name,
  const wxString& helptext,
  wxArtID artid)
{
  wxString use_name = name;
  wxBitmap bitmap;

  if (!artid.empty())
  {
    bitmap = wxArtProvider::GetBitmap(artid, wxART_MENU, wxSize(16, 15));
  }

  CheckStock(id, use_name, bitmap);

  wxMenuItem* item = new wxMenuItem(this, id, use_name, helptext);

  if (bitmap.IsOk())
  {
    item->SetBitmap(bitmap);
  }

  wxMenu::Append(item);

  return item;
}

bool exMenu::AppendEdit(bool add_invert)
{
  bool added = false;

  if (!(m_Style & MENU_IS_READ_ONLY) &&
       (m_Style & MENU_IS_SELECTED))
  {
    added = true;
    Append(wxID_CUT);
  }

  if (m_Style & MENU_IS_SELECTED)
  {
    added = true;
    Append(wxID_COPY);
  }

  if (!(m_Style & MENU_IS_READ_ONLY) &&
       (m_Style & MENU_CAN_PASTE))
  {
    added = true;
    Append(wxID_PASTE);
  }

  if (!(m_Style & MENU_IS_SELECTED) &&
      !(m_Style & MENU_IS_EMPTY))
  {
    added = true;
    Append(wxID_SELECTALL);
  }
  else
  {
    if (add_invert && !(m_Style & MENU_IS_EMPTY))
    {
      added = true;
      Append(ID_EDIT_SELECT_NONE, _("&Deselect All"));
    }
  }

  if (m_Style & MENU_ALLOW_CLEAR)
  {
    added = true;
    Append(wxID_CLEAR);
  }

  if (add_invert && !(m_Style & MENU_IS_EMPTY))
  {
    added = true;
    Append(ID_EDIT_SELECT_INVERT, _("&Invert"));
  }

  if (!(m_Style & MENU_IS_READ_ONLY) &&
       (m_Style & MENU_IS_SELECTED) &&
      !(m_Style & MENU_IS_EMPTY))
  {
    added = true;
    Append(wxID_DELETE);
  }

  return added;
}

void exMenu::AppendPrint()
{
  Append(wxID_PRINT_SETUP, exEllipsed(_("Page &Setup")));
  Append(wxID_PREVIEW);
  Append(wxID_PRINT);
}

exMenu* exMenu::AppendTools()
{
  exMenu* menuTool = new exMenu(*this);
  exMenu* menuReport = new exMenu(*this);

  for (
    map <int, const exToolInfo>::const_iterator it = exTool::GetToolInfo().begin();
    it != exTool::GetToolInfo().end();
    ++it)
  {
    if (!it->second.GetText().empty())
    {
      exMenu* menu = (it->second.GetIsBasic() ? menuTool: menuReport);
      menu->Append(it->first, it->second.GetText(), it->second.GetHelpText());
    }
  }

  menuTool->AppendSeparator();
  menuTool->AppendSubMenu(menuReport, _("&Report"));

  AppendSubMenu(menuTool, _("&Tools"));

  return menuTool;
}

int exPane::m_Total = 0;

#if wxUSE_STATUSBAR
BEGIN_EVENT_TABLE(exStatusBar, wxStatusBar)
  EVT_LEFT_DOWN(exStatusBar::OnMouse)
  EVT_LEFT_DCLICK(exStatusBar::OnMouse)
  EVT_MOTION(exStatusBar::OnMouse)
END_EVENT_TABLE()

exStatusBar::exStatusBar(
  exFrame* parent,
  wxWindowID id,
  long style,
  const wxString& name)
  : wxStatusBar(parent, id, style, name)
  , m_Frame(parent)
{
}

void exStatusBar::OnMouse(wxMouseEvent& event)
{
  bool found = false;

  for (int i = 0; i < GetFieldsCount() && !found; i++)
  {
    wxRect rect;

    if (GetFieldRect(i, rect))
    {
      if (rect.Contains(event.GetPosition()))
      {
        if (event.ButtonDClick())
        {
          m_Frame->StatusBarDoubleClicked(i, event.GetPosition());
        }
        else if (event.ButtonDown())
        {
          m_Frame->StatusBarClicked(i, event.GetPosition());
        }
        // Show tooltip if tooltip is available, and not yet tooltip presented.
        else if (event.Moving())
        {
          if (!m_Frame->m_Panes.empty())
          {
            const wxString tooltip =
              (GetToolTip() != NULL ? GetToolTip()->GetTip(): wxString(wxEmptyString));

            if (tooltip != m_Frame->GetPane(i).m_Helptext)
            {
              SetToolTip(m_Frame->GetPane(i).m_Helptext);
            }
          }
        }
        else
        {
          wxLogError(FILE_INFO("Unhandled"));
        }

        found = true;
      }
    }
  }

  event.Skip();
}
#endif //wxUSE_STATUSBAR

map<wxWindowID, wxArtID> exStockArt::m_StockArt;

exStockArt::exStockArt()
{
  if (m_StockArt.empty())
  {
    m_StockArt.insert(make_pair((wxWindowID)wxID_BACKWARD, wxART_GO_BACK));
    m_StockArt.insert(make_pair((wxWindowID)wxID_COPY, wxART_COPY));
    m_StockArt.insert(make_pair((wxWindowID)wxID_CUT, wxART_CUT));
    m_StockArt.insert(make_pair((wxWindowID)wxID_DELETE, wxART_DELETE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_DOWN, wxART_GO_DOWN));
    m_StockArt.insert(make_pair((wxWindowID)wxID_EXIT, wxART_QUIT));
    m_StockArt.insert(make_pair((wxWindowID)wxID_FIND, wxART_FIND));
    m_StockArt.insert(make_pair((wxWindowID)wxID_FORWARD, wxART_GO_FORWARD));
    m_StockArt.insert(make_pair((wxWindowID)wxID_HELP, wxART_HELP));
    m_StockArt.insert(make_pair((wxWindowID)wxID_HOME, wxART_GO_HOME));
    m_StockArt.insert(make_pair((wxWindowID)wxID_NEW, wxART_NEW));
    m_StockArt.insert(make_pair((wxWindowID)wxID_OPEN, wxART_FILE_OPEN));
    m_StockArt.insert(make_pair((wxWindowID)wxID_PASTE, wxART_PASTE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_PRINT, wxART_PRINT));
    m_StockArt.insert(make_pair((wxWindowID)wxID_REDO, wxART_REDO));
    m_StockArt.insert(make_pair((wxWindowID)wxID_REPLACE, wxART_FIND_AND_REPLACE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_SAVE, wxART_FILE_SAVE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_SAVEAS, wxART_FILE_SAVE_AS));
    m_StockArt.insert(make_pair((wxWindowID)wxID_UNDO, wxART_UNDO));
    m_StockArt.insert(make_pair((wxWindowID)wxID_UP, wxART_GO_UP));
    m_StockArt.insert(make_pair((wxWindowID)wxID_VIEW_DETAILS, wxART_REPORT_VIEW));
    m_StockArt.insert(make_pair((wxWindowID)wxID_VIEW_LIST, wxART_LIST_VIEW));
  }
}

void exStockArt::CheckStock(
  int id,
  wxString& stock_label,
  wxBitmap& bitmap,
  long flags,
  const wxSize& bitmap_size)
{
  if (wxIsStockID(id))
  {
    if (!stock_label.empty())
    {
      wxLogWarning(wxString::Format(
        "You specified a label: %s, though there is a stock label for it",
        stock_label.c_str()));
    }

    stock_label = wxGetStockLabel(id, flags);

    // Check if there is art for this id.
    map<wxWindowID, wxArtID>::const_iterator art_it = m_StockArt.find(id);

    if (art_it != m_StockArt.end())
    {
      if (bitmap.IsOk())
      {
        wxLogWarning(wxString::Format(
          "You specified art: %s, though there is stock art for it",
          stock_label.c_str()));
      }

      bitmap = wxArtProvider::GetBitmap(art_it->second, wxART_MENU, bitmap_size);
    }
  }
}

exToolBar::exToolBar(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxSize& bitmap_size,
  const wxString& name)
  : wxToolBar(parent, id, pos, size, style, name)
{
  SetToolBitmapSize(bitmap_size);
}

wxToolBarToolBase* exToolBar::AddTool(
  int toolId,
  const wxString& label,
  const wxBitmap& bitmap1,
  const wxString& shortHelpString,
  wxItemKind kind)
{
  wxString use_help = shortHelpString;
  wxBitmap use_bitmap = bitmap1;
  CheckStock(toolId, use_help, use_bitmap, wxSTOCK_NOFLAGS, GetToolBitmapSize());
  return wxToolBar::AddTool(toolId, label, use_bitmap, use_help, kind);
}

wxToolBarToolBase* exToolBar::AddTool(
  int toolId,
  const wxString& longHelpString,
  const wxString& label,
  const wxBitmap& bitmap1,
  const wxBitmap& bitmap2,
  wxItemKind kind,
  const wxString& shortHelpString,
  wxObject* clientData)
{
  wxString use_help = shortHelpString;
  wxBitmap use_bitmap = bitmap1;
  CheckStock(toolId, use_help, use_bitmap, wxSTOCK_NOFLAGS, GetToolBitmapSize());
  return wxToolBar::AddTool(
    toolId,
    label,
    use_bitmap,
    bitmap2,
    kind,
    use_help,
    longHelpString,
    clientData);
}

wxToolBarToolBase* exToolBar::AddCheckTool(
  int toolId,
  const wxString& label,
  const wxBitmap& bitmap1,
  const wxBitmap& bitmap2,
  const wxString& shortHelpString,
  const wxString& longHelpString,
  wxObject* clientData)
{
  wxString use_help = shortHelpString;
  wxBitmap use_bitmap = bitmap1;
  CheckStock(toolId, use_help, use_bitmap, wxSTOCK_NOFLAGS, GetToolBitmapSize());
  return wxToolBar::AddCheckTool(
    toolId,
    label,
    use_bitmap,
    bitmap2,
    use_help,
    longHelpString,
    clientData);
}
#endif // wxUSE_GUI
