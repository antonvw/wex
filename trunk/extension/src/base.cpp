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

#include <wx/tooltip.h> // for GetTip
#include <wx/extension/base.h>
#include <wx/extension/app.h>
#include <wx/extension/listview.h>
#include <wx/extension/stc.h>
#include <wx/extension/tool.h>

using namespace std;

#if wxUSE_GUI

#if wxUSE_STATUSBAR
wxExStatusBar* wxExFrame::m_StatusBar = NULL;
map<wxString, wxExPane> wxExFrame::m_Panes;
#endif

BEGIN_EVENT_TABLE(wxExFrame, wxFrame)
  EVT_CLOSE(wxExFrame::OnClose)
#if wxUSE_STATUSBAR
  EVT_UPDATE_UI(ID_EDIT_STATUS_BAR, wxExFrame::OnUpdateUI)
#endif
END_EVENT_TABLE()

wxExFrame::wxExFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style,
  const wxString& name)
  : wxFrame(parent, id, title, wxDefaultPosition, wxDefaultSize, style, name)
  , m_KeepPosAndSize(true)
{
  if (wxExApp::GetConfig("Frame/Maximized", 0l))
  {
    Maximize(true);
  }

  SetSize(
    wxExApp::GetConfig("Frame/X", 100),
    wxExApp::GetConfig("Frame/Y", 100),
    wxExApp::GetConfig("Frame/Width", 450),
    wxExApp::GetConfig("Frame/Height", 350));
}

wxExFrame::wxExFrame(wxWindow* parent,
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

wxExFrame::~wxExFrame()
{
#if wxUSE_STATUSBAR
  delete m_StatusBar;
#endif
}

wxExListView* wxExFrame::GetFocusedListView()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, wxExListView);
}

wxExSTC* wxExFrame::GetFocusedSTC()
{
  wxWindow* win = wxWindow::FindFocus();

  if (win == NULL)
  {
    return NULL;
  }

  return wxDynamicCast(win, wxExSTC);
}

#if wxUSE_STATUSBAR
const wxExPane wxExFrame::GetPane(int pane) const
{
  for (
    map<wxString, wxExPane>::const_iterator it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.m_No == pane)
    {
      return it->second;
    }
  }

  return wxExPane();
}

// This is a static method, so no const possible.
int wxExFrame::GetPaneField(const wxString& pane)
{
  map<wxString, wxExPane>::const_iterator it = m_Panes.find(pane);

  if (it != m_Panes.end())
  {
    return it->second.m_No;
  }

  return -1;
}
#endif // wxUSE_STATUSBAR

void wxExFrame::OnClose(wxCloseEvent& event)
{
  if (m_KeepPosAndSize)
  {
    // Set config values that might have changed.
    if (IsMaximized())
    {
      wxExApp::SetConfig("Frame/Maximized", 1);
    }
    else
    {
      wxExApp::SetConfig("Frame/Maximized", 0);
      const wxRect rect = GetRect();
      wxExApp::SetConfig("Frame/X", rect.GetX());
      wxExApp::SetConfig("Frame/Y", rect.GetY());
      wxExApp::SetConfig("Frame/Width", rect.GetWidth());
      wxExApp::SetConfig("Frame/Height", rect.GetHeight());
    }
  }

  event.Skip();
}

#if wxUSE_STATUSBAR
wxStatusBar* wxExFrame::OnCreateStatusBar(
  int number,
  long style,
  wxWindowID id,
  const wxString& name)
{
  m_StatusBar = new wxExStatusBar(this, id, style, name);
  m_StatusBar->SetFieldsCount(number);
  return m_StatusBar;
}
#endif

#if wxUSE_TOOLBAR
wxToolBar* wxExFrame::OnCreateToolBar(
  long style,
  wxWindowID id,
  const wxString& name)
{
  m_ToolBar = new wxExToolBar(this,
    id,
    wxDefaultPosition,
    wxDefaultSize,
    style,
    wxSize(16, 15),
    name);

  return m_ToolBar;
}
#endif

void wxExFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  wxExSTC* stc = GetFocusedSTC();
  if (stc == NULL) return;

  switch (event.GetId())
  {
#if wxUSE_STATUSBAR
  case ID_EDIT_STATUS_BAR: stc->UpdateStatusBar("PaneLines"); break;
#endif
  default:
    wxFAIL;
    break;
  }
}

bool wxExFrame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  long flags)
{
  wxExSTC* stc = GetFocusedSTC();

  if (stc != NULL)
  {
    // Remove link flags;
    const long new_flags = 0;
    return stc->Open(filename, line_number, match, new_flags);
  }

  return false;
}

#if wxUSE_STATUSBAR
void wxExFrame::SetupStatusBar(
  const vector<wxExPane>& panes,
  long style,
  wxWindowID id,
  const wxString& name)
{
  wxFrame::CreateStatusBar(panes.size(), style, id, name);

  int* styles = new int[panes.size()];
  int* widths = new int[panes.size()];

  for (
    vector<wxExPane>::const_iterator it = panes.begin();
    it != panes.end();
    ++it)
  {
    m_Panes[it->m_Name] = *it;
    styles[it->m_No] = it->GetStyle();
    widths[it->m_No] = it->GetWidth();
  }

  m_StatusBar->SetStatusStyles(panes.size(), styles);
  m_StatusBar->SetStatusWidths(panes.size(), widths);

  delete[] styles;
  delete[] widths;
}
#endif // wxUSE_STATUSBAR

#if wxUSE_STATUSBAR
void wxExFrame::StatusBarDoubleClicked(int field, const wxPoint& point)
{
  if (field == GetPaneField("PaneLines"))
  {
    wxExSTC* stc = GetSTC();
    if (stc != NULL) stc->GotoDialog();
  }
  else if (field == GetPaneField("PaneLexer"))
  {
    wxExSTC* stc = GetSTC();
    if (stc != NULL) stc->LexerDialog();
  }
  else if (field == GetPaneField("PaneFileType"))
  {
    wxExSTC* stc = GetSTC();
    if (stc != NULL) stc->FileTypeMenu();
  }
  else if (field == GetPaneField("PaneItems"))
  {
    wxExListView* list = GetListView();
    if (list != NULL) list->GotoDialog();
  }
  else
  {
    // Clicking on another field, do nothing. 
  }
}

// This is a static method, so you cannot call wxFrame::SetStatusText.
void wxExFrame::StatusText(const wxString& text, const wxString& pane)
{
  if (m_StatusBar == NULL || m_Panes.empty())
  {
    // You did not ask for a status bar, so ignore all.
    return;
  }

  const int field = GetPaneField(pane);

  if (field >= 0)
  {
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    m_StatusBar->SetStatusText(text, field);
  }
}

void wxExFrame::StatusText(const wxExFileName& filename, long flags)
{
  wxString text; // clear status bar for empty or not existing or not initialized file names

  if (filename.IsOk())
  {
    const wxString path = (flags & STAT_FULLPATH
      ? filename.GetFullPath(): filename.GetFullName());

    text += path;

    if (filename.GetStat().IsOk())
    {
      const wxString what = (flags & STAT_SYNC
        ? _("Synchronized"): _("Modified"));
      const wxString time = (flags & STAT_SYNC
        ? wxDateTime::Now().Format(): filename.GetStat().GetModificationTime());
      text += " " + what + " " + time;
    }
  }

  StatusText(text);
}

#endif // wxUSE_STATUSBAR

wxExManagedFrame::wxExManagedFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style,
  const wxString& name)
  : wxExFrame(parent, id, title, style, name)
{
  m_Manager.SetManagedWindow(this);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxExApp::GetPrinter()->SetParentWindow(this);
#endif
}

wxExManagedFrame::~wxExManagedFrame()
{
  m_Manager.UnInit();
}

void wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  wxASSERT(info.IsOk());

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}

int wxExPane::m_Total = 0;

#if wxUSE_STATUSBAR
BEGIN_EVENT_TABLE(wxExStatusBar, wxStatusBar)
  EVT_LEFT_DOWN(wxExStatusBar::OnMouse)
  EVT_LEFT_DCLICK(wxExStatusBar::OnMouse)
  EVT_MOTION(wxExStatusBar::OnMouse)
END_EVENT_TABLE()

wxExStatusBar::wxExStatusBar(
  wxExFrame* parent,
  wxWindowID id,
  long style,
  const wxString& name)
  : wxStatusBar(parent, id, style, name)
  , m_Frame(parent)
{
}

void wxExStatusBar::OnMouse(wxMouseEvent& event)
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
          wxFAIL;
        }

        found = true;
      }
    }
  }

  event.Skip();
}
#endif //wxUSE_STATUSBAR

wxExToolBar::wxExToolBar(wxWindow* parent,
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

wxToolBarToolBase* wxExToolBar::AddTool(
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

wxToolBarToolBase* wxExToolBar::AddTool(
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

wxToolBarToolBase* wxExToolBar::AddCheckTool(
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
