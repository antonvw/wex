////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.cpp
// Purpose:   Implementation of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/panel.h>
#include <wx/tokenzr.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

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
private:  
  void Expand();
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
};

wxExManagedFrame::wxExManagedFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  long style)
  : wxExFrame(parent, id, title, style)
  , m_FileHistory(maxFiles, wxID_FILE1)
  , m_ToolBar(new wxExToolBar(this))

{
  m_Manager.SetManagedWindow(this);
  AddToolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(new wxExFindToolBar(this), "FINDBAR", _("Findbar"));
  AddToolBarPane(new wxExOptionsToolBar(this), "OPTIONSBAR", _("Optionsbar"));
  AddToolBarPane(CreateExPanel(), "VIBAR");
  m_Manager.Update();
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_FileHistory.Save();
    event.Skip();});

  Bind(wxEVT_AUI_PANE_CLOSE, [=](wxAuiManagerEvent& event) {
    // TODO: wxAui should take care of this...
    wxAuiPaneInfo* info = event.GetPane();  
    info->BestSize(info->window->GetSize());
    info->Fixed();
    m_Manager.Update();
    info->Resizable();
    m_Manager.Update();
    });
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_FileHistory, event.GetId() - m_FileHistory.GetBaseId());},
    m_FileHistory.GetBaseId(), m_FileHistory.GetBaseId() + m_FileHistory.GetMaxFiles());
    
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_Manager.GetPane("FINDBAR").IsShown());}, ID_VIEW_FINDBAR);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_Manager.GetPane("OPTIONSBAR").IsShown());}, ID_VIEW_OPTIONSBAR);
  Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
    event.Check(m_Manager.GetPane("TOOLBAR").IsShown());}, ID_VIEW_TOOLBAR);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::list < wxString > l; 
    wxExFindReplaceData::Get()->SetFindStrings(l);}, ID_CLEAR_FINDS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExSTC::ConfigDialog(this,
      _("Editor Options"),
      wxExSTC::STC_CONFIG_MODELESS | wxExSTC::STC_CONFIG_WITH_APPLY,
      event.GetId());}, wxID_PREFERENCES);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_FileHistory.Clear();}, ID_CLEAR_FILES);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    int i = 0;
    wxString text;
    for (const auto& it : wxExFindReplaceData::Get()->GetFindStrings())
    {
      if (i++ == event.GetId() - ID_FIND_FIRST)
      {
        wxExSTC* stc = GetSTC();

        if (stc != NULL)
        {
          if (stc->FindNext(
            it,
            stc->GetVi().GetIsActive()? stc->GetVi().GetSearchFlags(): -1))
          {
            text = it;
          }
        }
        
        break;
      }
    }
    if (!text.empty())
    {
      wxExFindReplaceData::Get()->SetFindString(text);
    }}, ID_FIND_FIRST, ID_FIND_LAST);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("FINDBAR");}, ID_VIEW_FINDBAR);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("OPTIONSBAR");}, ID_VIEW_OPTIONSBAR);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    TogglePane("TOOLBAR");}, ID_VIEW_TOOLBAR);
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
      
    // Initially hide special bars.
    if (name == "FINDBAR" || name == "OPTIONSBAR" )
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
  // An ex panel starts with small static text for : or /, then
  // comes the ex ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  wxStaticText* text = new wxStaticText(panel, wxID_ANY, wxEmptyString);
  m_exTextCtrl = new wxExExTextCtrl(panel, this, text, wxID_ANY);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Expand());
  sizer->Add(m_exTextCtrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wxExManagedFrame::DoRecent(
  wxFileHistory& history, 
  size_t index, 
  long flags)
{
  const wxString file(history.GetHistoryFile(index));
  
  if (!file.empty())
  {
    OpenFile(file, 0, wxEmptyString, 0, flags);
  }
}

void wxExManagedFrame::GetExCommand(wxExEx* ex, const wxString& command)
{
  m_exTextCtrl->SetEx(ex, command);
  
  m_Manager.GetPane("VIBAR").Show();
  m_Manager.Update();
}

void wxExManagedFrame::HideExBar(int hide)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    if (hide == HIDE_BAR_FORCE || hide == HIDE_BAR_FORCE_FOCUS_STC ||
        (GetStatusBar() != NULL && GetStatusBar()->IsShown()))
    {
      m_Manager.GetPane("VIBAR").Hide();
      m_Manager.Update();
    }
    
    if ((hide == HIDE_BAR_FOCUS_STC || hide == HIDE_BAR_FORCE_FOCUS_STC) && 
         m_exTextCtrl != NULL && m_exTextCtrl->GetEx() != NULL)
    {
      m_exTextCtrl->GetEx()->GetSTC()->SetFocus();
    }
  }
}
  
void wxExManagedFrame::OnNotebook(wxWindowID id, wxWindow* page)
{
  wxExSTC* stc = wxDynamicCast(page, wxExSTC);

  if (stc != NULL)
  {
    if (stc->GetFileName().FileExists())
    {
      SetRecentFile(stc->GetFileName().GetFullPath());
    }
  }
  
  SetFindFocus(page);
}

void wxExManagedFrame::PrintEx(wxExEx* ex, const wxString& text)
{
  ex->Print(text);
}

bool wxExManagedFrame::OpenFile(
  const wxExFileName& filename,
  int line_number,
  const wxString& match,
  int col_number,
  long flags)
{
  if (wxExFrame::OpenFile(filename, line_number, match, col_number, flags))
  {
    SetRecentFile(filename.GetFullPath());
    return true;
  }

  return false;
}

void wxExManagedFrame::ShowExMessage(const wxString& text)
{
  if (GetStatusBar() != NULL && GetStatusBar()->IsShown())
  {
    HideExBar();
    GetStatusBar()->SetStatusText(text);
  }
  else
  {
    m_exTextCtrl->SetValue(text);
  }
}

void wxExManagedFrame::SyncAll()
{
  wxExSTC* stc = GetSTC();
  
  if (stc != NULL)
  {
    stc->Sync(wxConfigBase::Get()->ReadBool("AllowSync", true));
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
  , m_Commands(wxExListFromConfig("excommand"))
{
  m_CommandsIterator = m_Commands.begin();

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (event.GetUnicodeKey() != (wxChar)WXK_NONE)
    {
      if (event.GetKeyCode() == WXK_BACK)
      {
        m_Command = m_Command.Truncate(m_Command.size() - 1);
      }
      else if (event.GetKeyCode() != WXK_TAB)
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
      
      m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
      
      m_Controlr = false;
      m_UserInput = false;
    break;
    
    case WXK_CONTROL_R:
      m_Controlr = true;
      break;
  
    case WXK_TAB:
      Expand();
      break;
        
    default: Handle(event);
    }});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (
       m_UserInput && m_ex != NULL && IsFind())
    {
      m_ex->GetSTC()->PositionRestore();
      m_ex->GetSTC()->FindNext(
        GetValue(),
        m_ex->GetSearchFlags(),
        m_Prefix->GetLabel() == "/");
    }});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    if (m_ex == NULL)
    {
      return;
    }
    
    if (GetValue().empty())
    {
      m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
      return;
    }
    
    if (IsCommand())
    {
      if (m_ex->Command(wxString(m_Prefix->GetLabel() + GetValue()).ToStdString()))
      {
        const bool set_focus = 
          (GetValue() == "n" || GetValue() == "prev" || GetValue().StartsWith("!"));
            
        m_Commands.remove(GetValue());
        m_Commands.push_front(GetValue());
        m_CommandsIterator = m_Commands.begin();
  
        m_Frame->HideExBar(set_focus ? 
          wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC: wxExManagedFrame::HIDE_BAR_FOCUS_STC);
      }
    }
    else if (IsFind())
    {
      event.Skip();    
          
      if (m_UserInput)
      {
        m_ex->GetMacros().Record(wxString(m_Prefix->GetLabel() + GetValue()).ToStdString());
      }
      else 
      {
        if (!m_ex->Command(wxString(
          m_Prefix->GetLabel() + GetValue()).ToStdString()))
        {
          return;
        }
      }
      
      m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
    }
    else if (IsCalc())
    {
      if (m_UserInput)
      {
        m_ex->MacroRecord(m_Command.ToStdString());
      }
        
      if (m_ex->Command(wxString(
        m_Prefix->GetLabel() + GetValue()).ToStdString()))
      {
        m_Frame->HideExBar();
      }
    }});

  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    event.Skip();
    if (m_ex != NULL)
    {
      m_ex->GetSTC()->PositionSave();
    }});
}

wxExExTextCtrl::~wxExExTextCtrl()
{
  wxExListToConfig(m_Commands, "excommand");
}

void wxExExTextCtrl::Expand()
{
  if (m_ex != NULL && m_ex->GetSTC()->GetFileName().FileExists())
  {
    wxSetWorkingDirectory(m_ex->GetSTC()->GetFileName().GetPath());
  }
  
  std::vector<wxString> v;
  
  if (wxExAutoCompleteFileName(m_Command, v))
  {
    m_Command += v[0];
    AppendText(v[0]);
  }
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
    
void wxExExTextCtrl::SetEx(wxExEx* ex, const wxString& command) 
{
  m_Prefix->SetLabel(command.Left(1));
  const wxString range(command.Mid(1));
  
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
      
      if (m_ModeVisual && !current.StartsWith(range))
      {
        SetValue(range + current); 
      }
      else
      {
        SetValue(current);
      }
    }
    else
    {
      SetValue(range); 
    }
  }
    
  Show();
  SelectAll();
  SetFocus();
}
#endif // wxUSE_GUI
