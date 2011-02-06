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
#include <wx/extension/frd.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI
#if wxUSE_AUI

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

  AddToolBarPane(toolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(new wxExFindToolBar(this), "FINDBAR", _("Findbar"));
    
  CreateViPanel(m_viFindPrefix, m_viFind, "VIFINDBAR");
  CreateViPanel(m_viCommandPrefix, m_viCommand, "VICOMMANDBAR");
}

wxExManagedFrame::~wxExManagedFrame()
{
  m_Manager.UnInit();
}

bool wxExManagedFrame::AddToolBarPane(
  wxWindow* window, 
  const wxString& name,
  const wxString& caption)
{
  wxAuiPaneInfo pane;
  
  pane
    .LeftDockable(false)
    .RightDockable(false)
    .Name(name);

  // The real toolbar is at the top, others at bottom and initially hidden.  
  if (name == "TOOLBAR")
  {
    pane
      .Top()
      .ToolbarPane()
      .Caption(caption);
  }
  else
  {
    pane
      .Bottom()
      .CloseButton(false)
      .Hide()
      .DockFixed(true)
      .Movable(false)
      .CaptionVisible(false);
  }
  
  return m_Manager.AddPane(window, pane);
}

void wxExManagedFrame::CreateViPanel(
  wxStaticText*& statictext, 
  wxExTextCtrl*& text,
  const wxString& name)
{
  wxPanel* panel = new wxPanel(this);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  statictext = new wxStaticText(panel, wxID_ANY, wxEmptyString);
  text = new wxExTextCtrl(panel, this, statictext, wxID_ANY);
  
  sizer->AddGrowableCol(1);
  sizer->Add(statictext, wxSizerFlags().Expand());
  sizer->Add(text, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);
  
  AddToolBarPane(panel, name, _("vibar"));
}

void wxExManagedFrame::GetViCommand(wxExVi* vi, const wxString& command)
{
  if (command == ":")
  {
    GetViPaneCommand(
      m_viCommandPrefix, m_viCommand, "VICOMMANDBAR", vi, command);
  }
  else
  {
    // sync with frd data.
    m_viFind->SetValue(wxExFindReplaceData::Get()->GetFindString());
    
    GetViPaneCommand(
      m_viFindPrefix, m_viFind, "VIFINDBAR", vi, command);
  }
}
  
void wxExManagedFrame::GetViPaneCommand(
  wxStaticText* statictext,
  wxExTextCtrl* text,
  const wxString& pane,
  wxExVi* vi,
  const wxString& command)
{
  statictext->SetLabel(command);

  text->Show();
  text->SelectAll();
  text->SetFocus();
  text->SetVi(vi);
  
  m_Manager.GetPane(pane).Show();
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
  
  // Now get focus back to STC.
  // Problem is that you might have closed the STC document.
  wxExSTC* stc = GetSTC();
  
  if (stc != NULL)
  {
    stc->SetFocus();
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
  if (GetStatusBar()->IsShown())
  {
    GetStatusBar()->SetStatusText(text);
    
    m_Manager.GetPane("VICOMMANDBAR").Hide();
  }
  else
  {
    m_viCommandPrefix->SetLabel(text);
    m_viCommand->Hide();
  
    m_Manager.GetPane("VICOMMANDBAR").Show();
  }

  m_Manager.GetPane("VIFINDBAR").Hide();
  m_Manager.Update();
}

void wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  wxASSERT(info.IsOk());

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}

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
  if (m_vi != NULL)
  {
    if (m_StaticText->GetLabel() == ":")
    {
      if (m_vi->ExecCommand(GetValue()))
      {
        m_Frame->HideViBar();
      }
    }
    else
    {
      if (m_vi->FindCommand(m_StaticText->GetLabel(), GetValue()))
      {
        m_Frame->HideViBar();
      }
    }
  }
}

void wxExTextCtrl::OnKey(wxKeyEvent& event)
{
  const auto key = event.GetKeyCode();

  if (key == WXK_ESCAPE)
  {
    m_Frame->HideViBar();
  }
  else
  {
    event.Skip();
  }
}

#endif // wxUSE_AUI
#endif // wxUSE_GUI
