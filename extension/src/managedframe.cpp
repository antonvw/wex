////////////////////////////////////////////////////////////////////////////////
// Name:      managedframe.cpp
// Purpose:   Implementation of wxExManagedFrame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
#include <wx/extension/addressrange.h>
#include <wx/extension/defs.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

const int ID_REGISTER = wxNewId();

// Support class.
// Offers a text ctrl related to a ex object.
class wxExTextCtrl: public wxExFindTextCtrl
{
public:
  /// Constructor. Creates empty control.
  wxExTextCtrl(
    wxWindow* parent,
    wxExManagedFrame* frame,
    wxStaticText* prefix,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
    
  /// Destructor.
 ~wxExTextCtrl() {wxExListToConfig(m_Commands, "excommand");};
    
  /// Returns ex component.
  wxExEx* GetEx() {return m_ex;};
    
  /// Sets ex component.
  void SetEx(wxExEx* ex, const wxString& range);
private:  
  bool IsCalc() const {return m_Prefix->GetLabel() == "=";};
  bool IsCommand() const {return m_Prefix->GetLabel() == ":";};
  bool IsFind() const {return m_Prefix->GetLabel() == "/" || m_Prefix->GetLabel() == "?";};
  
  wxExManagedFrame* m_Frame;
  wxExEx* m_ex;
  wxStaticText* m_Prefix;
  bool m_ControlR;
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
  , m_ToggledPanes({
    {{"FINDBAR", _("&Findbar")}, ID_VIEW_LOWEST + 1},
    {{"OPTIONSBAR", _("&Optionsbar")}, ID_VIEW_LOWEST + 2},
    {{"TOOLBAR", _("&Toolbar")}, ID_VIEW_LOWEST + 3},
    {{"PROCESS", _("&Process")}, ID_VIEW_LOWEST + 4}})
  , m_OptionsBar(new wxExOptionsToolBar(this))
{
  m_Manager.SetManagedWindow(this);
  AddToolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(new wxExFindToolBar(this), "FINDBAR", _("Findbar"));
  AddToolBarPane(m_OptionsBar, "OPTIONSBAR", _("Optionsbar"));
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
    
    // If this pane is a toolbar pane, it might have a checkbox,
    // update that as well.
    m_OptionsBar->Update(info->name, false);
    });
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_FileHistory, event.GetId() - m_FileHistory.GetBaseId());},
    m_FileHistory.GetBaseId(), m_FileHistory.GetBaseId() + m_FileHistory.GetMaxFiles());

  for (auto it : m_ToggledPanes)
  {
    Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
      event.Check(m_Manager.GetPane(it.first.first).IsShown());}, it.second);
    Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      TogglePane(it.first.first);}, it.second);
  }
    
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
    wxExSTC* stc = GetSTC();
    if (stc != nullptr)
    {
      auto it = wxExFindReplaceData::Get()->GetFindStrings().begin();
      std::advance(it, event.GetId() - ID_FIND_FIRST);
    
      if (stc->FindNext(
        *it,
        stc->GetVi().GetIsActive()? stc->GetVi().GetSearchFlags(): -1))
      {
        wxExFindReplaceData::Get()->SetFindString(*it);
      }
    }}, ID_FIND_FIRST, ID_FIND_LAST);
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
  if (!caption.empty())
  {
    pane
      .Top()
      .ToolbarPane()
      .MinSize(-1, 30)
      .Caption(caption);
      
    // Initially hide special bars.
    if (name == "FINDBAR" || name == "OPTIONSBAR" )
    {
      pane.Hide();
    }
  }
  // otherwise (vi) fixed at the bottom and initially hidden.  
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
  SetFindFocus(nullptr);
  return true;
}

void wxExManagedFrame::AppendPanes(wxMenu* menu) const
{
#ifdef __WXMSW__
  menu->AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar\tCtrl+I"));
#endif

  for (auto it : m_ToggledPanes)
  {
    if (it.first.first == "PROCESS" && wxExProcess::GetShell() == nullptr)
    {
      continue;
    }
    
    menu->AppendCheckItem(it.second, it.first.second);
  }
}
  
wxPanel* wxExManagedFrame::CreateExPanel()
{
  // An ex panel starts with small static text for : or /, then
  // comes the ex ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  wxStaticText* text = new wxStaticText(panel, wxID_ANY, wxEmptyString);
  m_TextCtrl = new wxExTextCtrl(panel, this, text, wxID_ANY);
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Expand());
  sizer->Add(m_TextCtrl, wxSizerFlags().Expand());
  
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
  m_TextCtrl->SetEx(ex, command);
  ShowPane("VIBAR");
}

void wxExManagedFrame::HideExBar(int hide)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    if (hide == HIDE_BAR_FORCE || hide == HIDE_BAR_FORCE_FOCUS_STC ||
        (GetStatusBar() != nullptr && GetStatusBar()->IsShown()))
    {
      ShowPane("VIBAR", false);
    }
    
    if ((hide == HIDE_BAR_FOCUS_STC || 
         hide == HIDE_BAR_FORCE_FOCUS_STC) && 
         m_TextCtrl != nullptr && 
         m_TextCtrl->GetEx() != nullptr)
    {
      if (m_TextCtrl->GetValue().StartsWith("!") && 
        wxExAddressRange::GetProcess() != nullptr &&
        wxExAddressRange::GetProcess()->IsRunning())
      {
        wxExAddressRange::GetProcess()->GetShell()->SetFocus();
      }
      else
      {
        m_TextCtrl->GetEx()->GetSTC()->SetFocus();
      }
    }
  }
}
  
void wxExManagedFrame::OnNotebook(wxWindowID id, wxWindow* page)
{
  wxExSTC* stc = wxDynamicCast(page, wxExSTC);

  if (stc != nullptr)
  {
    if (stc->GetFileName().FileExists())
    {
      SetRecentFile(stc->GetFileName().GetFullPath());
    }
  }
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
  long flags,
  const wxString& command)
{
  if (wxExFrame::OpenFile(filename, line_number, match, col_number, flags, command))
  {
    SetRecentFile(filename.GetFullPath());
    return true;
  }

  return false;
}

void wxExManagedFrame::ShowExMessage(const wxString& text)
{
  if (GetStatusBar() != nullptr && GetStatusBar()->IsShown())
  {
    HideExBar();
    GetStatusBar()->SetStatusText(text);
  }
  else
  {
    m_TextCtrl->SetValue(text);
  }
}

bool wxExManagedFrame::ShowPane(const wxString& pane, bool show)
{
  wxAuiPaneInfo& info = m_Manager.GetPane(pane);

  if (!info.IsOk())
  {
    return false;
  }

  show ? info.Show(): info.Hide();

  m_Manager.Update();
  
  m_OptionsBar->Update(pane, show);
  
  return true;
}
  
void wxExManagedFrame::SyncAll()
{
  wxExSTC* stc = GetSTC();
  
  if (stc != nullptr)
  {
    stc->Sync(wxConfigBase::Get()->ReadBool("AllowSync", true));
  }
}

void wxExManagedFrame::SyncCloseAll(wxWindowID id)
{
  SetFindFocus(nullptr);
}

// Implementation of support class.

wxExTextCtrl::wxExTextCtrl(
  wxWindow* parent,
  wxExManagedFrame* frame,
  wxStaticText* prefix,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxExFindTextCtrl(parent, id, pos, size)
  , m_Frame(frame)
  , m_ex(nullptr)
  , m_ControlR(false)
  , m_ModeVisual(false)
  , m_UserInput(false)
  , m_Prefix(prefix)
  , m_Commands(wxExListFromConfig("excommand"))
{
  m_CommandsIterator = m_Commands.begin();

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (event.GetUnicodeKey() != WXK_NONE)
    {
      if (event.GetKeyCode() == WXK_BACK)
      {
        m_Command = m_Command.Truncate(m_Command.size() - 1);
      }
      else if (event.GetKeyCode() != WXK_TAB && event.GetKeyCode() != WXK_RETURN)
      {
        m_Command += event.GetUnicodeKey();
      }
    }
        
    switch (event.GetKeyCode())
    {
    case WXK_CONTROL_R: 
      m_ControlR = true; 
      break;

    case WXK_DOWN:
    case WXK_UP: 
      if (IsCommand())
      {
        wxExSetTextCtrlValue(this, event.GetKeyCode(), m_Commands, m_CommandsIterator);
      }
      else if (IsFind())
      {
        event.Skip();
      }
      break;
      
    case WXK_ESCAPE:
      if (m_ex != nullptr)
      {
        m_ex->GetSTC()->PositionRestore();
      }
      m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
      m_ControlR = false;
      m_UserInput = false;
    break;
    
    case WXK_TAB: {
      if (m_ex != nullptr && m_ex->GetSTC()->GetFileName().FileExists())
      {
        wxSetWorkingDirectory(m_ex->GetSTC()->GetFileName().GetPath());
      }
      std::vector<wxString> v;
      if (wxExAutoCompleteFileName(m_Command, v))
      {
        m_Command += v[0];
        AppendText(v[0]);
      }}
      break;

    default: {
      bool skip = true;
      if (event.GetKeyCode() != WXK_RETURN)
      {
        if (event.GetUnicodeKey() != (wxChar)WXK_NONE && m_ControlR)
        {
          skip = false;
          const wxChar c = event.GetUnicodeKey();
          wxCommandEvent event(wxEVT_MENU, ID_REGISTER);
          if (c == '%')
          {
            if (m_ex != nullptr) event.SetString(m_ex->GetSTC()->GetFileName().GetFullName());
          }
          else 
          {
            event.SetString(wxExEx::GetMacros().GetRegister(c));
          }
          if (!event.GetString().empty())
          {
            wxPostEvent(this, event);
          }
          if (m_ex != nullptr && m_ex->GetMacros().IsRecording())
          {
            m_Command += "\x12" + c;
          }
        }
        m_UserInput = true;
      }
      m_ControlR = false;
      if (skip)
      {
        event.Skip();
      }
    }}});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    WriteText(event.GetString());}, ID_REGISTER);
  
  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (m_UserInput && m_ex != nullptr && IsFind())
    {
      m_ex->GetSTC()->PositionRestore();
      m_ex->GetSTC()->FindNext(
        GetValue(),
        m_ex->GetSearchFlags(),
        m_Prefix->GetLabel() == "/");
    }});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    if (m_ex == nullptr || GetValue().empty())
    {
      m_Frame->HideExBar(wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC);
      return;
    }
    if (!m_ex->GetMacros().IsRecording())
    {
      m_Command = m_Prefix->GetLabel() + GetValue();
    }
    event.Skip();
    if (m_ex->Command(m_Command.ToStdString(), m_UserInput && IsFind()))
    {
      int focus = (IsFind() ? 
        wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC: 
        wxExManagedFrame::HIDE_BAR_FOCUS_STC);
      if (IsCommand())
      {
        if (GetValue() == "n" || GetValue() == "prev") focus = wxExManagedFrame::HIDE_BAR_FORCE;
        if (GetValue().StartsWith("!")) focus = wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC;
        m_Commands.remove(GetValue());
        m_Commands.push_front(GetValue());
        m_CommandsIterator = m_Commands.begin();
      }
      m_Frame->HideExBar(focus);}});

  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    event.Skip();
    if (m_ex != nullptr)
    {
      m_ex->GetSTC()->PositionSave();
    }});
}

void wxExTextCtrl::SetEx(wxExEx* ex, const wxString& command) 
{
  m_Prefix->SetLabel(command.Left(1));
  const wxString range(command.Mid(1));
  
  m_Command = m_Prefix->GetLabel();
  m_ControlR = false;
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
