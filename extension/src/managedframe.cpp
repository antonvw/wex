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
#include <wx/extension/debug.h>
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

enum
{
  TYPE_UNKNOWN,
  TYPE_CALC,
  TYPE_COMMAND,
  TYPE_FIND,
};

// Support class.
// Offers a text ctrl related to a ex object.
class wxExTextCtrl: public wxTextCtrl
{
public:
  // Constructor. Creates empty control.
  wxExTextCtrl(
    wxWindow* parent,
    wxExManagedFrame* frame,
    wxStaticText* prefix,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
    
  // Returns ex component.
  wxExEx* GetEx() {return m_ex;};
    
  // Sets ex component.
  // Returns false if command not supported.
  bool SetEx(wxExEx* ex, const std::string& command);
private:  
  int GetType() const {return GetType(m_Prefix->GetLabel().ToStdString());};
  int GetType(const std::string& command) const {
    if (command.empty()) return TYPE_UNKNOWN;
    switch (command[0])
    {
      case ':': return TYPE_COMMAND; break;
      case '=': return TYPE_CALC; break;
      case '/':
      case '?': return TYPE_FIND; break;
      default: return TYPE_UNKNOWN;};};
  wxExManagedFrame* m_Frame;
  wxExEx* m_ex = nullptr;
  wxStaticText* m_Prefix;
  bool m_ControlR = false;
  bool m_ModeVisual = false;
  bool m_UserInput = false;
  
  wxExTextCtrlInput m_Calcs;
  wxExTextCtrlInput m_Commands;
  wxString m_Command;
};

wxExManagedFrame::wxExManagedFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  long style)
  : wxExFrame(parent, id, title, style)
  , m_Debug(new wxExDebug(this))
  , m_FileHistory(maxFiles, wxID_FILE1)
  , m_FindBar(new wxExFindToolBar(this))
  , m_OptionsBar(new wxExOptionsToolBar(this))
  , m_ToolBar(new wxExToolBar(this))
  , m_ToggledPanes({
    {{"FINDBAR", _("&Findbar")}, ID_VIEW_LOWEST + 1},
    {{"OPTIONSBAR", _("&Optionsbar")}, ID_VIEW_LOWEST + 2},
    {{"TOOLBAR", _("&Toolbar")}, ID_VIEW_LOWEST + 3},
    {{"PROCESS", _("&Process")}, ID_VIEW_LOWEST + 4}})
{
  m_Manager.SetManagedWindow(this);
  AddToolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(m_FindBar,  "FINDBAR", _("Findbar"));
  AddToolBarPane(m_OptionsBar, "OPTIONSBAR", _("Optionsbar"));
  AddToolBarPane(CreateExPanel(), "VIBAR");
  m_Manager.Update();
  
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
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_FileHistory.Save();
    event.Skip();});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_FileHistory, event.GetId() - m_FileHistory.GetBaseId());},
    m_FileHistory.GetBaseId(), m_FileHistory.GetBaseId() + m_FileHistory.GetMaxFiles());

  for (const auto& it : m_ToggledPanes)
  {
    Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
      event.Check(m_Manager.GetPane(it.first.first).IsShown());}, it.second);
    Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      TogglePane(it.first.first);}, it.second);
  }
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxExFindReplaceData::Get()->SetFindStrings(std::list < wxString > {});}, ID_CLEAR_FINDS);
    
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
      if (stc->FindNext(*it, stc->GetVi().GetIsActive()? stc->GetVi().GetSearchFlags(): -1))
      {
        wxExFindReplaceData::Get()->SetFindString(*it);
      }
    }}, ID_FIND_FIRST, ID_FIND_LAST);
}

wxExManagedFrame::~wxExManagedFrame()
{
  delete m_Debug;
  
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

  menu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));

  for (const auto& it : m_ToggledPanes)
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

bool wxExManagedFrame::GetExCommand(wxExEx* ex, const std::string& command)
{
  return ShowPane("VIBAR") && m_TextCtrl->SetEx(ex, command);
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
      m_TextCtrl->GetEx()->GetSTC()->SetFocus();
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

void wxExManagedFrame::PrintEx(wxExEx* ex, const std::string& text)
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
  : wxTextCtrl(parent, id, wxEmptyString, pos, size, wxTE_PROCESS_ENTER)
  , m_Frame(frame)
  , m_Prefix(prefix)
  , m_Calcs("excalc")
  , m_Commands("excommand")
{
  SetFont(wxConfigBase::Get()->ReadObject(_("Text font"), 
    wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

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

      case WXK_TAB: {
        if (m_ex != nullptr && m_ex->GetSTC()->GetFileName().FileExists())
        {
          wxSetWorkingDirectory(m_ex->GetSTC()->GetFileName().GetPath());
        }
        std::vector<std::string> v;
        if (wxExAutoCompleteFileName(m_Command.ToStdString(), v))
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
              m_Command << "\x12" << c;
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

  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    switch (event.GetKeyCode())
    {
      case WXK_DOWN:
      case WXK_END:
      case WXK_HOME:
      case WXK_PAGEDOWN:
      case WXK_PAGEUP:
      case WXK_UP:
        if ((event.GetKeyCode() == WXK_HOME || event.GetKeyCode() == WXK_END) && !event.ControlDown())
        {
          event.Skip();
        }
        else switch (GetType())
        {
          case TYPE_CALC: m_Calcs.Set(event.GetKeyCode(), this); break;
          case TYPE_COMMAND: m_Commands.Set(event.GetKeyCode(), this); break;
          case TYPE_FIND: wxExFindReplaceData::Get()->m_FindStrings.Set(event.GetKeyCode(), this); break;
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
      
      default: event.Skip();
        break;
      }});
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    WriteText(event.GetString());}, ID_REGISTER);
  
  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (m_UserInput && m_ex != nullptr && GetType() == TYPE_FIND)
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
    if (m_ex->Command(m_Command.ToStdString(), m_UserInput && GetType() == TYPE_FIND))
    {
      int focus = (GetType() == TYPE_FIND ? 
        wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC: 
        wxExManagedFrame::HIDE_BAR_FOCUS_STC);
      switch (GetType())
      {
        case TYPE_CALC: m_Calcs.Set(this); break;
        case TYPE_COMMAND:
          if (GetValue() == "n" || GetValue() == "prev") focus = wxExManagedFrame::HIDE_BAR_FORCE;
          if (GetValue().StartsWith("!")) focus = wxExManagedFrame::HIDE_BAR_FORCE_FOCUS_STC;
          m_Commands.Set(this);
          break;
        case TYPE_FIND:
          wxExFindReplaceData::Get()->SetFindString(GetValue());
          break;
      }
      m_Frame->HideExBar(focus);}});

  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    event.Skip();
    if (m_ex != nullptr)
    {
      m_ex->GetSTC()->PositionSave();
    }});
}

bool wxExTextCtrl::SetEx(wxExEx* ex, const std::string& command) 
{
  if (command.empty()) return false;

  m_UserInput = false;
  const wxString range(command.substr(1));
  m_ModeVisual = !range.empty();

  switch (GetType(command))
  {
    case TYPE_CALC: SetValue(m_Calcs.Get()); break;
    case TYPE_COMMAND:
      if (!m_Commands.Get().empty())
      {
        SetValue(m_ModeVisual && !m_Commands.Get().StartsWith(range) ? 
          range + m_Commands.Get(): m_Commands.Get()); 
      }
      else
      {
        SetValue(range); 
      }
      break;
    case TYPE_FIND: SetValue(!m_ModeVisual ? ex->GetSTC()->GetFindString(): wxString()); break;
    case TYPE_UNKNOWN: return false;
  }
    
  m_Prefix->SetLabel(command.substr(0, 1));
  m_Command = m_Prefix->GetLabel();
  m_ControlR = false;
  m_ex = ex;

  Show();
  SelectAll();
  SetFocus();

  return true;
}
#endif // wxUSE_GUI
