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
    
  wxPanel* vipanel = new wxPanel(this);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  m_viStaticText = new wxStaticText(vipanel, wxID_ANY, wxEmptyString);
  m_viTextCtrl = new wxExTextCtrl(vipanel, this, m_viStaticText);
  
  sizer->AddGrowableCol(1);
  sizer->Add(m_viStaticText, wxSizerFlags().Expand());
  sizer->Add(m_viTextCtrl, wxSizerFlags().Expand());
  
  vipanel->SetSizerAndFit(sizer);
  
  m_Manager.AddPane(vipanel,
    wxAuiPaneInfo().Bottom().Floatable(false).Name("VIBAR").CaptionVisible(false));
    
  m_Manager.GetPane("VIBAR").Hide();
}

wxExManagedFrame::~wxExManagedFrame()
{
  m_Manager.UnInit();
}

void wxExManagedFrame::GetViCommand(wxExVi* vi, const wxString& command)
{
  m_viStaticText->SetLabel(command);

  m_viTextCtrl->Show();
  m_viTextCtrl->SelectAll();
  m_viTextCtrl->SetFocus();
  m_viTextCtrl->SetVi(vi);
  
  m_Manager.GetPane("VIBAR").Show();
  m_Manager.Update();
}
  
void wxExManagedFrame::HideViBar()
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    m_Manager.GetPane("VIBAR").Hide();
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
  m_viStaticText->SetLabel(text);
  m_viTextCtrl->Hide();
  
  m_Manager.GetPane("VIBAR").Show();
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
