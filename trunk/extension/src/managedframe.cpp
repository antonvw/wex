////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.cpp
// Purpose:   Implementation of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Created:   2010-04-11
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>

#if wxUSE_GUI

#if wxUSE_AUI
BEGIN_EVENT_TABLE(wxExManagedFrame, wxExFrame)
  EVT_MENU(wxID_PREFERENCES, wxExManagedFrame::OnCommand)
  EVT_MENU_RANGE(ID_VIEW_LOWEST, ID_VIEW_HIGHEST, wxExManagedFrame::OnCommand)
  EVT_UPDATE_UI_RANGE(
    ID_VIEW_LOWEST, ID_VIEW_HIGHEST, wxExManagedFrame::OnUpdateUI)
END_EVENT_TABLE()

wxExManagedFrame::wxExManagedFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style)
  : wxExFrame(parent, id, title, style)
{
  m_Manager.SetManagedWindow(this);

  wxExToolBar* toolBar = new wxExToolBar(this);

  toolBar->AddControls();

  DoAddControl(toolBar);

  m_Manager.AddPane(toolBar,
    wxAuiPaneInfo().Top().ToolbarPane().Name("TOOLBAR").Caption(_("Toolbar")));

  m_Manager.AddPane(new wxExFindToolBar(this),
    wxAuiPaneInfo().Bottom().ToolbarPane().Name("FINDBAR").Caption(_("Findbar")));
}

wxExManagedFrame::~wxExManagedFrame()
{
  m_Manager.UnInit();
}

void wxExManagedFrame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
    case wxID_PREFERENCES:
      wxExSTC::ConfigDialog(this,
        _("Editor Options"),
        wxExSTC::STC_CONFIG_MODELESS | 
        wxExSTC::STC_CONFIG_SIMPLE |
        wxExSTC::STC_CONFIG_WITH_APPLY,
        event.GetId());
    break;

    case ID_VIEW_FINDBAR: TogglePane("FINDBAR"); break;
    case ID_VIEW_TOOLBAR: TogglePane("TOOLBAR"); break;

    case ID_VIEW_MENUBAR:
      if (GetMenuBar()->IsShown())
      {
        SetMenuBar(NULL);
      }
      break;

    case ID_VIEW_STATUSBAR:
      if (GetStatusBar() != NULL)
      {
        GetStatusBar()->Show(!GetStatusBar()->IsShown());
        SendSizeEvent();
      }
      break;

    default:
      wxFAIL;
  }
}

void wxExManagedFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
    case ID_VIEW_FINDBAR:
      event.Check(GetManager().GetPane("FINDBAR").IsShown());
    break;

    case ID_VIEW_TOOLBAR:
      event.Check(GetManager().GetPane("TOOLBAR").IsShown());
    break;

    case ID_VIEW_MENUBAR:
      if (GetMenuBar() != NULL)
      {
        event.Check(GetMenuBar()->IsShown());
      }
      else
      {
        event.Check(false);
      }
    break;

    case ID_VIEW_STATUSBAR:
      if (GetStatusBar() != NULL)
      {
        event.Check(GetStatusBar()->IsShown());
      }
      else
      {
        event.Check(false);
      }
      break;

    default:
      wxFAIL;
  }
}

void wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  wxASSERT(info.IsOk());

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}
#endif // wxUSE_AUI
#endif // wxUSE_GUI
