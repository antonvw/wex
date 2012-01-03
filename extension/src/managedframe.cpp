////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.cpp
// Purpose:   Implementation of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Created:   2010-04-11
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
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
class wxExViTextCtrl: public wxTextCtrl
{
public:
  /// Constructor. Creates empty control.
  wxExViTextCtrl(
    wxWindow* parent,
    wxExManagedFrame* frame,
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
  void OnFocus(wxFocusEvent& event);
  void OnKey(wxKeyEvent& event);
private:  
  wxExManagedFrame* m_Frame;
  wxExVi* m_vi;
  bool m_UserInput;
  bool m_Found;

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

  m_ToolBar = new wxExToolBar(this);
  m_ToolBar->AddControls();

  AddToolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(new wxExFindToolBar(this), "FINDBAR", _("Findbar"));
  AddToolBarPane(CreateViPanel(), "VIBAR");
  
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
      .Row(10)
      .CaptionVisible(false);
  }
  
  return m_Manager.AddPane(window, pane);
}

bool wxExManagedFrame::AllowClose(wxWindowID id, wxWindow* page)
{
  // The page will be closed, so do not update find focus now.
  SetFindFocus(NULL);
  return true;
}

wxPanel* wxExManagedFrame::CreateViPanel()
{
  // A vi panel starts with small static text for : or /, then
  // comes the vi ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  m_viTextPrefix = new wxStaticText(panel, wxID_ANY, wxEmptyString);
  m_viTextCtrl = new wxExViTextCtrl(panel, this, wxID_ANY);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(m_viTextPrefix, wxSizerFlags().Expand());
  sizer->Add(m_viTextCtrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wxExManagedFrame::GetViCommand(wxExVi* vi, const wxString& command)
{
  m_viTextPrefix->SetLabel(command);
  m_viTextCtrl->SetVi(vi);
  
  m_Manager.GetPane("VIBAR").Show();
  m_Manager.Update();
}

bool wxExManagedFrame::GetViCommandIsFind() const
{
  return GetViCommandIsFindNext() || GetViCommandIsFindPrevious();
}

bool wxExManagedFrame::GetViCommandIsFindNext() const
{
  return m_viTextPrefix->GetLabel() == "/";
}
  
bool wxExManagedFrame::GetViCommandIsFindPrevious() const
{
  return m_viTextPrefix->GetLabel() == "?";
}

void wxExManagedFrame::HideViBar(bool set_focus)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    m_Manager.GetPane("VIBAR").Hide();
    m_Manager.Update();
    
    if (set_focus &&  m_viTextCtrl != NULL && m_viTextCtrl->GetVi() != NULL)
    {
      m_viTextCtrl->GetVi()->GetSTC()->SetFocus();
    }
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

void wxExManagedFrame::OnNotebook(wxWindowID id, wxWindow* page)
{
  SetFindFocus(page);
}

void wxExManagedFrame::ShowViMessage(const wxString& text)
{
  if (GetStatusBar()->IsShown())
  {
    GetStatusBar()->SetStatusText(text);
    
    HideViBar();
  }
  else
  {
    m_viTextPrefix->SetLabel(text);
    m_viTextCtrl->Hide();
    m_viTextCtrl->GetVi()->GetSTC()->SetFocus();
  
    m_Manager.GetPane("VIBAR").Show();
    m_Manager.Update();
  }
}

void wxExManagedFrame::SyncCloseAll(wxWindowID id)
{
  SetFindFocus(NULL);
}

void wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  wxASSERT(info.IsOk());

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
}

// Implementation of support class.

BEGIN_EVENT_TABLE(wxExViTextCtrl, wxTextCtrl)
  EVT_CHAR(wxExViTextCtrl::OnKey)
  EVT_SET_FOCUS(wxExViTextCtrl::OnFocus)
  EVT_TEXT(wxID_ANY, wxExViTextCtrl::OnCommand)
  EVT_TEXT_ENTER(wxID_ANY, wxExViTextCtrl::OnEnter)
END_EVENT_TABLE()

wxExViTextCtrl::wxExViTextCtrl(
  wxWindow* parent,
  wxExManagedFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxTextCtrl(parent, id, wxEmptyString, pos, size, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
  , m_vi(NULL)
  , m_UserInput(false)
  , m_Found(false)
{
}

void wxExViTextCtrl::OnCommand(wxCommandEvent& event)
{
  event.Skip();
  
  if (m_UserInput && m_vi != NULL && m_Frame->GetViCommandIsFind())
  {
    m_vi->GetSTC()->PositionRestore();
    m_Found = m_vi->GetSTC()->FindNext(
      GetValue(),
      m_vi->GetSearchFlags(),
      m_Frame->GetViCommandIsFindNext());
  }
}

void wxExViTextCtrl::OnEnter(wxCommandEvent& event)
{
  if (!m_Frame->GetViCommandIsFind())
  {
    if (m_vi != NULL)
    {
      if (m_vi->ExecCommand(GetValue()))
      {
        wxConfigBase::Get()->Write("vicommand", GetValue());
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
        
        if (m_vi != NULL)
        {
          m_vi->MacroRecord(GetValue());
        }
      }
    }
    else if (m_vi != NULL)
    {
      m_Found = m_vi->FindNext(GetValue(), m_Frame->GetViCommandIsFindNext());
    }
      
    m_Frame->HideViBar();
  }
}

void wxExViTextCtrl::OnFocus(wxFocusEvent& event)
{
  event.Skip();

  if (m_vi != NULL)
  {
    m_vi->GetSTC()->PositionSave();
  }
}

void wxExViTextCtrl::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

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
    if (key != WXK_RETURN)
    {
      m_UserInput = true;
    }
    else
    {
      m_UserInput = m_Found;
    }
    
    event.Skip();
  }
}

void wxExViTextCtrl::SetVi(wxExVi* vi) 
{
  m_UserInput = false;

  m_vi = vi;
  
  Show();
    
  if (m_Frame->GetViCommandIsFind())
  {
    if (!m_vi->GetSTC()->GetSelectedText().empty())
    {
      SetValue(m_vi->GetSTC()->GetSelectedText());
      wxExFindReplaceData::Get()->SetFindString(GetValue());
    }
    else
    {
      SetValue(wxExFindReplaceData::Get()->GetFindString());
    }
  }
  else
  {
    SetValue(wxConfigBase::Get()->Read("vicommand", wxEmptyString));
  }
    
  SelectAll();
  SetFocus();
}

#endif // wxUSE_GUI
