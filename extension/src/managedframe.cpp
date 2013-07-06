////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.cpp
// Purpose:   Implementation of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
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
class wxExExTextCtrl: public wxExFindTextCtrl
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
    
  /// Destructor.
 ~wxExExTextCtrl();
    
  /// Returns ex component.
  wxExEx* GetEx() {return m_ex;};
    
  /// Sets ex component.
  void SetEx(wxExEx* ex, const wxString& range);
protected:
  void OnChar(wxKeyEvent& event);
  void OnCommand(wxCommandEvent& event);
  void OnEnter(wxCommandEvent& event);
  void OnFocus(wxFocusEvent& event);
private:  
  void Handle(wxKeyEvent& event);
  bool IsCalc() const {return 
    m_Prefix->GetLabel() == "=";};
  bool IsCommand() const {return 
    m_Prefix->GetLabel() == ":";};
  bool IsFind() const {return 
    m_Prefix->GetLabel() == "/" || m_Prefix->GetLabel() == "?";};
  
  wxExManagedFrame* m_Frame;
  wxExEx* m_ex;
  wxStaticText* m_Prefix;
  bool m_Controlr;
  bool m_ModeVisual;
  bool m_UserInput;
  
  wxString m_Command;
  
  std::list < wxString > m_Commands;
  std::list < wxString >::const_iterator m_CommandsIterator;

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
      .Resizable()
      .Caption(caption);
      
    // Initially hide findbar as well.
    if (name == "FINDBAR")
    {
      pane.Hide();
    }
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
  m_exTextPrefix->SetLabel(command.Left(1));
  m_exTextCtrl->SetEx(ex, command.Mid(1));
  
  m_Manager.GetPane("VIBAR").Show();
  m_Manager.Update();
}

void wxExManagedFrame::HideExBar(bool set_focus)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    m_Manager.GetPane("VIBAR").Hide();
    m_Manager.Update();
    
    if (set_focus && m_exTextCtrl != NULL && m_exTextCtrl->GetEx() != NULL)
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
  HideExBar();
  
  if (GetStatusBar() != NULL && GetStatusBar()->IsShown())
  {
    GetStatusBar()->SetStatusText(text);
  }
  else
  {
    wxLogMessage(text);
  }
}

void wxExManagedFrame::SyncCloseAll(wxWindowID id)
{
  SetFindFocus(NULL);
}

bool wxExManagedFrame::TogglePane(const wxString& pane)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  if (!info.IsOk())
  {
    return false;
  }

  info.IsShown() ? info.Hide(): info.Show();

  m_Manager.Update();
  
  return true;
}

// Implementation of support class.

BEGIN_EVENT_TABLE(wxExExTextCtrl, wxExFindTextCtrl)
  EVT_CHAR(wxExExTextCtrl::OnChar)
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
  : wxExFindTextCtrl(parent, id, pos, size)
  , m_Frame(frame)
  , m_ex(NULL)
  , m_Controlr(false)
  , m_ModeVisual(false)
  , m_UserInput(false)
  , m_Prefix(prefix)
{
  wxStringTokenizer tkz(wxConfigBase::Get()->Read("excommand"),
    wxExGetFieldSeparator());

  while (tkz.HasMoreTokens())
  {
    const wxString val = tkz.GetNextToken();
    m_Commands.push_front(val);
  }

  m_CommandsIterator = m_Commands.begin();
  
  // Next does not work.
  //AutoCompleteFileNames();
}

wxExExTextCtrl::~wxExExTextCtrl()
{
  const int commandsSaveInConfig = 25;
  
  wxString values;
  int items = 0;

  for (
    std::list < wxString >::reverse_iterator it = m_Commands.rbegin();
    it != m_Commands.rend() && items < commandsSaveInConfig;
    ++it)
  {
    values += *it + wxExGetFieldSeparator();
    items++;
  }

  wxConfigBase::Get()->Write("excommand", values);
}

void wxExExTextCtrl::Handle(wxKeyEvent& event)
{
  bool skip = true;
  
  if (event.GetKeyCode() != WXK_RETURN)
  {
    if (
      IsCalc() &&
      event.GetUnicodeKey() != (wxChar)WXK_NONE &&
      m_Controlr)
    {
      skip = false;
      
      const wxChar c = event.GetUnicodeKey();
    
      switch (c)
      {
      case '\"':
        AppendText(wxExClipboardGet()); break;
          
      default:
        if (
           m_ex != NULL &&
          !m_ex->GetMacros().GetRegister(c).empty())
        {
          AppendText(m_ex->GetMacros().GetRegister(c));
        }
      }
    }
    
    m_UserInput = true;
  }
  
  if (skip)
  {
    event.Skip();
  }
  
  m_Controlr = false;
}
    
void wxExExTextCtrl::OnChar(wxKeyEvent& event)
{
  if (event.GetUnicodeKey() != (wxChar)WXK_NONE)
  {
    if (event.GetKeyCode() == WXK_BACK)
    {
      m_Command = m_Command.Truncate(m_Command.size() - 1);
    }
    else
    {
      m_Command += event.GetUnicodeKey();
    }
  }
      
  const int key = event.GetKeyCode();

  switch (key)
  {
  case WXK_UP: 
  case WXK_DOWN:
    if (IsCommand())
    {
      wxExSetTextCtrlValue(this, key, m_Commands, m_CommandsIterator);
    }
    else if (IsFind())
    {
      event.Skip();
    }
    break;
    
  case WXK_ESCAPE:
    if (m_ex != NULL)
    {
      m_ex->GetSTC()->PositionRestore();
    }
    
    m_Frame->HideExBar();
    
    m_Controlr = false;
    m_UserInput = false;
  break;
  
  case WXK_CONTROL_R:
    m_Controlr = true;
    break;

  default: Handle(event);
  }
}

void wxExExTextCtrl::OnCommand(wxCommandEvent& event)
{
  event.Skip();
  
  if (
     m_UserInput && m_ex != NULL && IsFind())
  {
    m_ex->GetSTC()->PositionRestore();
    m_ex->GetSTC()->FindNext(
      GetValue(),
      wxSTC_FIND_REGEXP | wxFR_MATCHCASE,
      m_Prefix->GetLabel() == "/",
      m_ModeVisual);
  }
}

void wxExExTextCtrl::OnEnter(wxCommandEvent& event)
{
  if (m_ex == NULL)
  {
    return;
  }
  
  if (GetValue().empty())
  {
    m_Frame->HideExBar();
    return;
  }
  
  if (IsCommand())
  {
    if (m_ex->Command(m_Prefix->GetLabel() + GetValue()))
    {
      const bool set_focus = 
        (GetValue() == "n" || GetValue() == "prev");
          
      m_Commands.remove(GetValue());
      m_Commands.push_front(GetValue());
      m_CommandsIterator = m_Commands.begin();

      m_Frame->HideExBar(!set_focus);
    }
  }
  else if (IsFind())
  {
    event.Skip();    
        
    if (m_UserInput)
    {
      m_ex->GetMacros().Record(m_Prefix->GetLabel() + GetValue());
    }
    else 
    {
      m_ex->Command(m_Prefix->GetLabel() + GetValue());
    }
    
    m_Frame->HideExBar();
  }
  else if (IsCalc())
  {
    if (m_UserInput)
    {
      m_ex->MacroRecord(m_Command);
    }
      
    m_ex->Command(m_Prefix->GetLabel() + GetValue());
    
    m_Frame->HideExBar();
  }
  else
  {
    wxFAIL;
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

void wxExExTextCtrl::SetEx(wxExEx* ex, const wxString& range) 
{
  m_Command.clear();
  m_Controlr = false;
  m_ModeVisual = !range.empty();
  m_UserInput = false;
  m_ex = ex;
  
  SetValue(wxEmptyString);
  
  if (IsFind())
  {
    if (!m_ModeVisual)
    {
      SetValue(m_ex->GetSTC()->GetFindString());
    }
  }
  else if (IsCommand())
  {
    if (m_Commands.begin() != m_Commands.end())
    {
      const wxString current(*m_Commands.begin());
      
      SetValue((!current.StartsWith(range) ? range: wxString(wxEmptyString)) + current);
    }
  }
    
  Show();
  SelectAll();
  SetFocus();
}
#endif // wxUSE_GUI
