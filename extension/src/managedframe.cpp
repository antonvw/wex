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
#include <wx/wxcrt.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vi.h>

#if wxUSE_GUI

// Support class.
// Offers a text ctrl related to a vi object.
class wxExViFindCtrl: public wxTextCtrl
{
public:
  /// Constructor. Creates empty control.
  wxExViFindCtrl(
    wxWindow* parent,
    wxExManagedFrame* frame,
    wxStaticText* text,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
    
  /// Returns vi component.
  wxExVi* GetVi() {return m_vi;};
    
  /// Sets vi component.
  void SetVi(wxExVi* vi);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnEnter(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
private:  
  wxExManagedFrame* m_Frame;
  wxExVi* m_vi;
  const wxStaticText* m_StaticText;
  bool m_UserInput;

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
  
  m_Manager.Update();
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

  // If the toolbar has a caption, it is at the top, 
  // otherwise fixed at the bottom and initially hidden.  
  if (!caption.empty())
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
  wxExViFindCtrl*& victrl,
  const wxString& name)
{
  // A vi panel starts with small static text for : or /, then
  // comes the vi ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  statictext = new wxStaticText(panel, wxID_ANY, wxEmptyString);
  victrl = new wxExViFindCtrl(panel, this, statictext, wxID_ANY);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(statictext, wxSizerFlags().Expand());
  sizer->Add(victrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);
  
  AddToolBarPane(panel, name);
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
  wxExViFindCtrl* victrl,
  const wxString& pane,
  wxExVi* vi,
  const wxString& command)
{
  statictext->SetLabel(command);

  victrl->SetVi(vi);
  
  m_Manager.GetPane(pane).Show();
  m_Manager.Update();
}
    
void wxExManagedFrame::HideViBar()
{
  if (m_Manager.GetPane("VIFINDBAR").IsShown())
  {
    m_Manager.GetPane("VIFINDBAR").Hide();
    m_Manager.Update();
    
    m_viFind->GetVi()->GetSTC()->SetFocus();
  }
  else if (m_Manager.GetPane("VICOMMANDBAR").IsShown())
  {
    m_Manager.GetPane("VICOMMANDBAR").Hide();
    m_Manager.Update();
    
    m_viCommand->GetVi()->GetSTC()->SetFocus();
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

BEGIN_EVENT_TABLE(wxExViFindCtrl, wxTextCtrl)
  EVT_CHAR(wxExViFindCtrl::OnKey)
  EVT_TEXT(wxID_ANY, wxExViFindCtrl::OnCommand)
  EVT_TEXT_ENTER(wxID_ANY, wxExViFindCtrl::OnEnter)
END_EVENT_TABLE()

wxExViFindCtrl::wxExViFindCtrl(
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
  , m_UserInput(false)
{
}

void wxExViFindCtrl::OnCommand(wxCommandEvent& event)
{
  event.Skip();
  
  if (m_UserInput && m_vi != NULL && m_StaticText->GetLabel() != ":")
  {
    m_vi->GetSTC()->PositionRestore();
    m_vi->GetSTC()->FindNext(
      GetValue(),
      m_vi->GetSearchFlags(),
      m_StaticText->GetLabel() == '/');
  }
}

void wxExViFindCtrl::OnEnter(wxCommandEvent& event)
{
  event.Skip();
  
  if (m_StaticText->GetLabel() == ":")
  {
    if (m_vi != NULL)
    {
      if (m_vi->ExecCommand(GetValue()))
      {
        m_Frame->HideViBar();
      }
    }
  }
  else
  {
    if (m_UserInput)
    {
      if (!GetValue().empty())
      {
        wxExFindReplaceData::Get()->SetFindString(GetValue());
      }
    }
    else if (m_vi != NULL)
    {
      m_vi->GetSTC()->FindNext(
        GetValue(),
        m_vi->GetSearchFlags(),
        m_StaticText->GetLabel() == '/');
    }
      
    m_Frame->HideViBar();
  }
}

void wxExViFindCtrl::OnKey(wxKeyEvent& event)
{
  const auto key = event.GetKeyCode();

  if (key == WXK_ESCAPE)
  {
    if (m_vi != NULL)
    {
      m_vi->GetSTC()->PositionRestore();
    }
    
    m_Frame->HideViBar();
    
    m_UserInput = false;
  }
  else
  {
    if (!m_UserInput)
    {
      if (m_vi != NULL)
      {
        m_vi->GetSTC()->PositionSave();
      }
        
      m_UserInput = true;
    }
    
    event.Skip();
  }
}

void wxExViFindCtrl::SetVi(wxExVi* vi) 
{
  if (vi->GetSTC()->IsBeingDeleted())
  {
    wxLogStatus("invalid vi");
    m_vi = NULL;
  }
  else
  {
    m_vi = vi;
    m_UserInput = false;
  
    Show();
    SelectAll();
    SetFocus();
  }
}

#endif // wxUSE_GUI
