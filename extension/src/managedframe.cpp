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
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

// Support class.
// Offers a text ctrl related to a ex object.
class wxExExTextCtrl: public wxTextCtrl
{
public:
  /// Constructor. Creates empty control.
  wxExExTextCtrl(
    wxWindow* parent,
    wxExManagedFrame* frame,
    wxStaticText* prefix,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
    
  /// Returns ex component.
  wxExEx* GetEx() {return m_ex;};
    
  /// Sets ex component.
  void SetEx(wxExEx* ex);
protected:
  void OnCommand(wxCommandEvent& event);
  void OnEnter(wxCommandEvent& event);
  void OnFocus(wxFocusEvent& event);
  void OnKey(wxKeyEvent& event);
private:  
  wxExManagedFrame* m_Frame;
  wxExEx* m_ex;
  wxStaticText* m_Prefix;
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
  AddToolBarPane(CreateExPanel(), "VIBAR");
  
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

wxPanel* wxExManagedFrame::CreateExPanel()
{
  // A ex panel starts with small static text for : or /, then
  // comes the ex ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  m_exTextPrefix = new wxStaticText(panel, wxID_ANY, wxEmptyString);
  m_exTextCtrl = new wxExExTextCtrl(panel, this, m_exTextPrefix, wxID_ANY);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(m_exTextPrefix, wxSizerFlags().Expand());
  sizer->Add(m_exTextCtrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wxExManagedFrame::GetExCommand(wxExEx* ex, const wxString& command)
{
  m_exTextPrefix->SetLabel(command);
  m_exTextCtrl->SetEx(ex);
  
  m_Manager.GetPane("VIBAR").Show();
  m_Manager.Update();
}

void wxExManagedFrame::HideExBar(bool set_focus)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    m_Manager.GetPane("VIBAR").Hide();
    m_Manager.Update();
    
    if (set_focus &&  m_exTextCtrl != NULL && m_exTextCtrl->GetEx() != NULL)
    {
      m_exTextCtrl->GetEx()->GetSTC()->SetFocus();
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

void wxExManagedFrame::ShowExMessage(const wxString& text)
{
  if (GetStatusBar()->IsShown())
  {
    GetStatusBar()->SetStatusText(text);
    
    HideExBar();
  }
  else
  {
    m_exTextPrefix->SetLabel(text);
    m_exTextCtrl->Hide();
    m_exTextCtrl->GetEx()->GetSTC()->SetFocus();
  
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

BEGIN_EVENT_TABLE(wxExExTextCtrl, wxTextCtrl)
  EVT_CHAR(wxExExTextCtrl::OnKey)
  EVT_SET_FOCUS(wxExExTextCtrl::OnFocus)
  EVT_TEXT(wxID_ANY, wxExExTextCtrl::OnCommand)
  EVT_TEXT_ENTER(wxID_ANY, wxExExTextCtrl::OnEnter)
END_EVENT_TABLE()

wxExExTextCtrl::wxExExTextCtrl(
  wxWindow* parent,
  wxExManagedFrame* frame,
  wxStaticText* prefix,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxTextCtrl(parent, id, wxEmptyString, pos, size, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
  , m_ex(NULL)
  , m_UserInput(false)
  , m_Found(false)
  , m_Prefix(prefix)
{
}

void wxExExTextCtrl::OnCommand(wxCommandEvent& event)
{
  event.Skip();
  
  if (m_UserInput && m_ex != NULL && m_Prefix->GetLabel() != ":")
  {
    m_ex->GetSTC()->PositionRestore();
    
    m_Found = m_ex->GetSTC()->FindNext(
      GetValue(),
      wxSTC_FIND_REGEXP | wxFR_MATCHCASE,
      m_Prefix->GetLabel() == "/");
  }
}

void wxExExTextCtrl::OnEnter(wxCommandEvent& event)
{
  if (m_Prefix->GetLabel() == ":")
  {
    if (m_ex != NULL)
    {
      if (m_ex->Command(":" + GetValue()))
      {
        wxConfigBase::Get()->Write("excommand", GetValue());
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
        
        if (m_ex != NULL)
        {
          m_ex->MacroRecord(GetValue());
        }
      }
    }
    else if (m_ex != NULL)
    {
      m_Found = m_ex->Command(m_Prefix->GetLabel() + GetValue());
    }
      
    m_Frame->HideExBar();
  }
}

void wxExExTextCtrl::OnFocus(wxFocusEvent& event)
{
  event.Skip();

  if (m_ex != NULL)
  {
    m_ex->GetSTC()->PositionSave();
  }
}

void wxExExTextCtrl::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

  if (key == WXK_ESCAPE)
  {
    if (m_ex != NULL)
    {
      m_ex->GetSTC()->PositionRestore();
    }
    
    m_Frame->HideExBar();
    
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

void wxExExTextCtrl::SetEx(wxExEx* ex) 
{
  m_UserInput = false;

  m_ex = ex;
  
  Show();
    
  if (m_Prefix->GetLabel() != ":")
  {
    if (!m_ex->GetSTC()->GetSelectedText().empty())
    {
      SetValue(m_ex->GetSTC()->GetSelectedText());
      wxExFindReplaceData::Get()->SetFindString(GetValue());
    }
    else
    {
      SetValue(wxExFindReplaceData::Get()->GetFindString());
    }
  }
  else
  {
    SetValue(wxConfigBase::Get()->Read("excommand", wxEmptyString));
  }
    
  SelectAll();
  SetFocus();
}

#endif // wxUSE_GUI
