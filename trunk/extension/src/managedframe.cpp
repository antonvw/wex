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
#include <wx/extension/util.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI

// Support class.
// Offers a text ctrl related to a vi object.
class wxExTextCtrl: public wxTextCtrl
{
public:
  /// Constructor. Fills the combobox box with values 
  /// from FindReplace from config.
  wxExTextCtrl(
    wxWindow* parent,
    wxExManagedFrame* frame,
    wxStaticText* text,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
    
  /// Sets callback.
  void SetVi(wxExVi* vi) {m_vi = vi;};
private:
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
  
  wxExManagedFrame* m_Frame;
  wxExVi* m_vi;
  wxStaticText* m_StaticText;

  DECLARE_EVENT_TABLE()
};

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
    
  wxPanel* find_panel = new wxPanel(this);
  
  wxFlexGridSizer* find_sizer = new wxFlexGridSizer(2);
  m_viFindPrefix = new wxStaticText(find_panel, wxID_ANY, wxEmptyString);
  m_viFind = new wxExTextCtrl(find_panel, this, m_viFindPrefix);
  
  find_sizer->AddGrowableCol(1);
  find_sizer->Add(m_viFindPrefix, wxSizerFlags().Expand());
  find_sizer->Add(m_viFind, wxSizerFlags().Expand());
  
  find_panel->SetSizerAndFit(find_sizer);
  
  wxPanel* command_panel = new wxPanel(this);
  
  wxFlexGridSizer* command_sizer = new wxFlexGridSizer(2);
  m_viCommandPrefix = new wxStaticText(command_panel, wxID_ANY, wxEmptyString);
  m_viCommand = new wxExTextCtrl(command_panel, this, m_viCommandPrefix);
  
  command_sizer->AddGrowableCol(1);
  command_sizer->Add(m_viCommandPrefix, wxSizerFlags().Expand());
  command_sizer->Add(m_viCommand, wxSizerFlags().Expand());
  
  command_panel->SetSizerAndFit(command_sizer);
  
  m_Manager.AddPane(find_panel,
    wxAuiPaneInfo().Bottom().Floatable(false).Name("VIFINDBAR").CaptionVisible(false));
  m_Manager.AddPane(command_panel,
    wxAuiPaneInfo().Bottom().Floatable(false).Name("VICOMMANDBAR").CaptionVisible(false));
    
  m_Manager.GetPane("VIFINDBAR").Hide();
  m_Manager.GetPane("VICOMMANDBAR").Hide();
}

wxExManagedFrame::~wxExManagedFrame()
{
  m_Manager.UnInit();
}

void wxExManagedFrame::GetViCommand(wxExVi* vi, const wxString& command)
{
  if (command == ":")
  {
    m_viCommandPrefix->SetLabel(command);

    m_viCommand->Show();
    m_viCommand->SelectAll();
    m_viCommand->SetFocus();
    m_viCommand->SetVi(vi);
  
    m_Manager.GetPane("VICOMMANDBAR").Show();
  }
  else
  {
    m_viFindPrefix->SetLabel(command);

    m_viFind->Show();
    m_viFind->SelectAll();
    m_viFind->SetFocus();
    m_viFind->SetVi(vi);
  
    m_Manager.GetPane("VIFINDBAR").Show();
  }
    
  m_Manager.Update();
}
  
void wxExManagedFrame::HideViBar()
{
  if (m_Manager.GetPane("VIFINDBAR").IsShown())
  {
    m_Manager.GetPane("VIFINDBAR").Hide();
    m_Manager.Update();
  }
  
  if (m_Manager.GetPane("VICOMMANDBAR").IsShown())
  {
    m_Manager.GetPane("VICOMMANDBAR").Hide();
    m_Manager.Update();
  }
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

    default:
      wxFAIL;
  }
}

void wxExManagedFrame::OnUpdateUI(wxUpdateUIEvent& event)
{
  switch (event.GetId())
  {
    case ID_VIEW_FINDBAR:
      event.Check(m_Manager.GetPane("FINDBAR").IsShown());
    break;

    case ID_VIEW_TOOLBAR:
      event.Check(m_Manager.GetPane("TOOLBAR").IsShown());
    break;

    default:
      wxFAIL;
  }
}

void wxExManagedFrame::ShowViMessage(const wxString& text)
{
  m_viCommandPrefix->SetLabel(text);
  m_viCommand->Hide();
  
  m_Manager.GetPane("VICOMMANDBAR").Show();
  m_Manager.Update();
}

void wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  wxASSERT(info.IsOk());

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}
#endif // wxUSE_AUI

// Implementation of support class.

BEGIN_EVENT_TABLE(wxExTextCtrl, wxTextCtrl)
  EVT_CHAR(wxExTextCtrl::OnKey)
  EVT_TEXT_ENTER(wxID_ANY, wxExTextCtrl::OnCommand)
END_EVENT_TABLE()

wxExTextCtrl::wxExTextCtrl(
  wxWindow* parent,
  wxExManagedFrame* frame,
  wxStaticText* text,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxTextCtrl(parent, id, wxEmptyString, pos, size, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
  , m_vi(NULL)
  , m_StaticText(text)
{
}

void wxExTextCtrl::OnCommand(wxCommandEvent& event)
{
  if (m_StaticText->GetLabel() == ":")
  {
    if (m_vi->ExecCommand(GetValue()))
    {
      m_Frame->HideViBar();
      m_vi->GetSTC()->SetFocus();
    }
  }
  else
  {
    if (m_vi->FindCommand(m_StaticText->GetLabel(), GetValue()))
    {
      m_Frame->HideViBar();
      m_vi->GetSTC()->SetFocus();
    }
  }
}

void wxExTextCtrl::OnKey(wxKeyEvent& event)
{
  const auto key = event.GetKeyCode();

  if (key == WXK_ESCAPE)
  {
    if (m_vi != NULL)
    {
      m_Frame->HideViBar();
      m_vi->GetSTC()->SetFocus();
    }
  }
  else
  {
    event.Skip();
  }
}

#endif // wxUSE_GUI
