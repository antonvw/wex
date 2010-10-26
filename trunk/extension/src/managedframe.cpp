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

// Support class.
// Offers a combobox related to a vi object.
class wxExComboBox: public wxComboBox
{
public:
  /// Constructor. Fills the combobox box with values 
  /// from FindReplace from config.
  wxExComboBox(
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
  m_viComboBox = new wxExComboBox(vipanel, this, m_viStaticText);
  
  sizer->AddGrowableCol(1);
  sizer->Add(m_viStaticText, wxSizerFlags().Expand());
  sizer->Add(m_viComboBox, wxSizerFlags().Expand());
  
  vipanel->SetSizerAndFit(sizer);
  wxExComboBoxFromList(m_viComboBox, wxExListFromConfig("VIBAR"));
  
  m_Manager.AddPane(vipanel,
    wxAuiPaneInfo().Bottom().Floatable(false).Name("VIBAR").CaptionVisible(false));
    
  m_Manager.GetPane("VIBAR").Hide();
}

wxExManagedFrame::~wxExManagedFrame()
{
  const auto l = wxExComboBoxToList(m_viComboBox, 25);
  wxExListToConfig(l, "VIBAR");
  m_Manager.UnInit();
}

void wxExManagedFrame::GetViCommand(wxExVi* vi, const wxString& command)
{
  m_viStaticText->SetLabel(command);

  m_viComboBox->Show();
  m_viComboBox->SelectAll();
  m_viComboBox->SetFocus();
  m_viComboBox->SetVi(vi);
  
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
  m_viComboBox->Hide();
  
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

BEGIN_EVENT_TABLE(wxExComboBox, wxComboBox)
  EVT_CHAR(wxExComboBox::OnKey)
  EVT_TEXT_ENTER(wxID_ANY, wxExComboBox::OnCommand)
END_EVENT_TABLE()

wxExComboBox::wxExComboBox(
  wxWindow* parent,
  wxExManagedFrame* frame,
  wxStaticText* text,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxComboBox(parent, id, wxEmptyString, pos, size, 0, NULL, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
  , m_vi(NULL)
  , m_StaticText(text)
{
}

void wxExComboBox::OnCommand(wxCommandEvent& event)
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

void wxExComboBox::OnKey(wxKeyEvent& event)
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
