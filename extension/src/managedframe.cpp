////////////////////////////////////////////////////////////////////////////////
// Name:      managed_frame.cpp
// Purpose:   Implementation of wex::managed_frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/panel.h>
#include <wex/managedframe.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/defs.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/process.h>
#include <wex/shell.h>
#include <wex/stc.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/vi-macros.h>
#include <wex/vi-macros-mode.h>

const int ID_REGISTER = wxWindow::NewControlId();

namespace wex
{
  // Support class.
  // Offers a text ctrl related to a ex object.
  class textctrl: public wxTextCtrl
  {
  public:
    // Constructor. Creates empty control.
    textctrl(
      managed_frame* frame,
      wxStaticText* prefix,
      const window_data& data);
      
    // Returns ex component.
    auto* GetEx() {return m_ex;};
      
    // Sets ex component.
    // Returns false if command not supported.
    bool SetEx(ex* ex, const std::string& command);
  private:
    textctrl_input& TCI() {
      switch (m_Command.Type())
      {
        case ex_command::type::CALC: return m_Calcs;
        case ex_command::type::EXEC: return m_Execs;
        case ex_command::type::FIND_MARGIN: return m_FindMargins;
        default: return m_Commands;
      }};
    ex_command m_Command;
    managed_frame* m_Frame;
    ex* m_ex {nullptr};
    wxStaticText* m_Prefix;
    bool m_ControlR {false}, m_ModeVisual {false}, m_UserInput {false};
    textctrl_input 
      m_Calcs {ex_command::type::CALC},
      m_Commands {ex_command::type::COMMAND},
      m_Execs {ex_command::type::EXEC},
      m_FindMargins {ex_command::type::FIND_MARGIN};
  };
};

wex::managed_frame::managed_frame(size_t maxFiles, const window_data& data)
  : frame(data)
  , m_Debug(new debug(this))
  , m_FileHistory(maxFiles, wxID_FILE1)
  , m_FindBar(new find_toolbar(this))
  , m_OptionsBar(new options_toolbar(this))
  , m_ToolBar(new toolbar(this))
  , m_ToggledPanes({
    {{"FINDBAR", _("&Findbar").ToStdString()}, ID_VIEW_LOWEST + 1},
    {{"OPTIONSBAR", _("&Optionsbar").ToStdString()}, ID_VIEW_LOWEST + 2},
    {{"TOOLBAR", _("&Toolbar").ToStdString()}, ID_VIEW_LOWEST + 3},
    {{"PROCESS", _("&Process").ToStdString()}, ID_VIEW_LOWEST + 4}})
{
  m_Manager.SetManagedWindow(this);
  AddToolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  AddToolBarPane(m_FindBar,  "FINDBAR", _("Findbar"));
  AddToolBarPane(m_OptionsBar, "OPTIONSBAR", _("Optionsbar"));
  AddToolBarPane(CreateExPanel(), "VIBAR");
  m_Manager.Update();
  
  Bind(wxEVT_AUI_PANE_CLOSE, [=](wxAuiManagerEvent& event) {
    wxAuiPaneInfo* info = event.GetPane();
    info->BestSize(info->window->GetSize()).Fixed().Resizable();
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
    m_FileHistory.GetBaseId(), 
    m_FileHistory.GetBaseId() + m_FileHistory.GetMaxFiles());

  for (const auto& it : m_ToggledPanes)
  {
    Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
      event.Check(m_Manager.GetPane(it.first.first).IsShown());}, it.second);
    Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      TogglePane(it.first.first);}, it.second);
  }
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    find_replace_data::Get()->SetFindStrings(std::list < std::string > {});}, ID_CLEAR_FINDS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    stc::ConfigDialog(
      window_data().
        Id(wxID_PREFERENCES).
        Parent(this).
        Title(_("Editor Options").ToStdString()).
        Button(wxAPPLY | wxOK | wxCANCEL));}, wxID_PREFERENCES);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_FileHistory.Clear();}, ID_CLEAR_FILES);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* stc = GetSTC(); stc != nullptr)
    {
      auto it = find_replace_data::Get()->GetFindStrings().begin();
      std::advance(it, event.GetId() - ID_FIND_FIRST);
      if (const std::string text(*it); stc->FindNext(text, 
        stc->GetVi().GetIsActive()? stc->GetVi().GetSearchFlags(): -1))
      {
        find_replace_data::Get()->SetFindString(text);
      }
    }}, ID_FIND_FIRST, ID_FIND_LAST);
}

wex::managed_frame::~managed_frame()
{
  delete m_Debug;
  
  m_Manager.UnInit();
}

bool wex::managed_frame::AddToolBarPane(
  wxWindow* window, 
  const std::string& name,
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

bool wex::managed_frame::AllowClose(wxWindowID id, wxWindow* page)
{
  // The page will be closed, so do not update find focus now.
  SetFindFocus(nullptr);
  return true;
}

void wex::managed_frame::AppendPanes(wxMenu* menu) const
{
#ifdef __WXMSW__
  menu->AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar\tCtrl+I"));
#endif

  menu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));

  for (const auto& it : m_ToggledPanes)
  {
    if (it.first.first == "PROCESS" && process::GetShell() == nullptr)
    {
      continue;
    }
    
    menu->AppendCheckItem(it.second, it.first.second);
  }
}
  
wxPanel* wex::managed_frame::CreateExPanel()
{
  // An ex panel starts with small static text for : or /, then
  // comes the ex ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  wxStaticText* text = new wxStaticText(panel, wxID_ANY, " ");
  m_TextCtrl = new textctrl(this, text, window_data().Parent(panel));
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Center());
  sizer->Add(m_TextCtrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wex::managed_frame::DoRecent(
  const file_history& history, 
  size_t index, 
  stc_data::window_flags flags)
{
  if (const path file(history.GetHistoryFile(index)); !file.Path().empty())
  {
    OpenFile(file, stc_data().Flags(flags));
  }
}

bool wex::managed_frame::GetExCommand(ex* ex, const std::string& command)
{
  return ShowPane("VIBAR") && m_TextCtrl->SetEx(ex, command);
}

void wex::managed_frame::HideExBar(int hide)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    if (hide == HIDE_BAR_FORCE || hide == HIDE_BAR_FORCE_FOCUS_STC ||
        (GetStatusBar() != nullptr && GetStatusBar()->IsShown()))
    {
      ShowPane("VIBAR", false);
    }
    
    if ((hide == HIDE_BAR_FOCUS_STC || hide == HIDE_BAR_FORCE_FOCUS_STC) && 
         m_TextCtrl != nullptr && 
         m_TextCtrl->GetEx() != nullptr)
    {
      m_TextCtrl->GetEx()->GetSTC()->SetFocus();
    }
  }
}
  
void wex::managed_frame::OnNotebook(wxWindowID id, wxWindow* page)
{
  if (auto* stc = wxDynamicCast(page, wex::stc); stc != nullptr)
  {
    SetRecentFile(stc->GetFileName());
  }
}

wex::stc* wex::managed_frame::OpenFile(
  const path& file,
  const stc_data& data)
{
  if (auto* stc = frame::OpenFile(file, data); stc != nullptr)
  {
    SetRecentFile(file);
    return stc;
  }

  return nullptr;
}

void wex::managed_frame::PrintEx(ex* ex, const std::string& text)
{
  ex->Print(text);
}

void wex::managed_frame::SetRecentFile(const path& path) 
{
  m_FileHistory.Add(path);
}

void wex::managed_frame::ShowExMessage(const std::string& text)
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

bool wex::managed_frame::ShowPane(const std::string& pane, bool show)
{
  if (wxAuiPaneInfo& info = m_Manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else 
  {
    show ? info.Show(): info.Hide();
    m_Manager.Update();
    m_OptionsBar->Update(pane, show);
    return true;
  }
}
  
void wex::managed_frame::SyncAll()
{
  if (auto* stc = GetSTC(); stc != nullptr)
  {
    stc->Sync(config("AllowSync").get(true));
  }
}

void wex::managed_frame::SyncCloseAll(wxWindowID id)
{
  SetFindFocus(nullptr);
}

// Implementation of support class.

wex::textctrl::textctrl(
  managed_frame* frame,
  wxStaticText* prefix,
  const window_data& data)
  : wxTextCtrl(
      data.Parent(), 
      data.Id(), 
      wxEmptyString, 
      data.Pos(), 
      data.Size(), 
      data.Style() | wxTE_PROCESS_ENTER)
  , m_Frame(frame)
  , m_Prefix(prefix)
{
  SetFont(config(_("Text font")).get(
    wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (event.GetUnicodeKey() != WXK_NONE)
    {
      if (event.GetKeyCode() == WXK_BACK)
      {
        if (!m_Command.empty()) m_Command.pop_back();
      }
      else if (event.GetKeyCode() != WXK_TAB)
      {
        m_Command.Append(event.GetUnicodeKey());
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
          path::Current(m_ex->GetSTC()->GetFileName().GetPath());
        }

        // studio not yet: [[maybe_unused]]
        if (const auto& [r, e, v] = autocomplete_filename(m_Command.Command());
          r)
        {
          m_Command.Append(e);
          AppendText(e);
        }}
        break;

      default: {
        bool skip = true;
        if (event.GetKeyCode() != WXK_RETURN)
        {
          if (event.GetUnicodeKey() != (wxChar)WXK_NONE && m_ControlR)
          {
            skip = false;
            const char c = event.GetUnicodeKey();
            wxCommandEvent event(wxEVT_MENU, ID_REGISTER);
            if (c == '%')
            {
              if (m_ex != nullptr) event.SetString(m_ex->GetSTC()->GetFileName().GetFullName());
            }
            else 
            {
              event.SetString(ex::GetMacros().GetRegister(c));
            }
            if (!event.GetString().empty())
            {
              wxPostEvent(this, event);
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
        if ((event.GetKeyCode() == WXK_HOME || event.GetKeyCode() == WXK_END) && 
            !event.ControlDown())
        {
          event.Skip();
        }
        else if (m_Command.Type() == ex_command::type::FIND)
        {
          find_replace_data::Get()->m_FindStrings.Set(event.GetKeyCode(), this); 
        }
        else
        {
          TCI().Set(event.GetKeyCode(), this); 
        }
        break;
        
      case WXK_ESCAPE:
        if (m_ex != nullptr)
        {
          m_ex->GetSTC()->PositionRestore();
        }
        m_Frame->HideExBar(managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
        m_ControlR = false;
        m_UserInput = false;
        break;

#ifdef __WXOSX__      
      // FIXME. See also toolbar.
      case WXK_RETURN:
        {
        wxCommandEvent event(wxEVT_TEXT_ENTER, m_windowId);
        event.SetEventObject( this );
        event.SetString( GetValue() );
        HandleWindowEvent(event);
        }
        break;
#endif
      
      default: 
        event.Skip();
        break;
      }});
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    WriteText(event.GetString());}, ID_REGISTER);
  
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    event.Skip();
    if (m_ex != nullptr)
    {
      m_ex->GetSTC()->PositionSave();
    }});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (m_UserInput && m_ex != nullptr && m_Command.Type() == ex_command::type::FIND)
    {
      m_ex->GetSTC()->PositionRestore();
      m_ex->GetSTC()->FindNext(
        GetValue().ToStdString(),
        m_ex->GetSearchFlags(),
        m_Prefix->GetLabel() == "/");
    }});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    if (m_ex == nullptr || GetValue().empty())
    {
      m_Frame->HideExBar(managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
      return;
    }

    m_Command.Command(m_Prefix->GetLabel().ToStdString() + GetValue().ToStdString());
    m_Command.IsHandled(m_UserInput && m_Command.Type() == ex_command::type::FIND);

    if (m_Command.Exec())
    {
      int focus = (m_Command.Type() == ex_command::type::FIND ? 
        managed_frame::HIDE_BAR_FORCE_FOCUS_STC: 
        managed_frame::HIDE_BAR_FOCUS_STC);

      if (m_Command.Type() == ex_command::type::FIND)
      {
        find_replace_data::Get()->SetFindString(GetValue().ToStdString());
      }
      else
      {
        TCI().Set(this); 

        if (m_Command.Type() == ex_command::type::COMMAND)
        {
          if (
            GetValue() == "gt" || 
            GetValue() == "n" || 
            GetValue() == "prev" || 
            GetValue().StartsWith("ta")) 
          {
            focus = managed_frame::HIDE_BAR_FORCE;
          }
          else if (GetValue().find("!") == 0) 
          {
            focus = managed_frame::HIDE_BAR;
          }
        }
      }

      m_Frame->HideExBar(focus);
    }});
}

bool wex::textctrl::SetEx(ex* ex, const std::string& command) 
{
  if (command.empty()) return false;

  m_ex = ex;
  m_UserInput = false;
  const std::string range(command.substr(1));
  m_ModeVisual = !range.empty();
  m_Prefix->SetLabel(command.substr(0, 1));
  m_Command = ex_command(ex->GetCommand()).Command(command);
  m_ControlR = false;

  switch (m_Command.Type())
  {
    case ex_command::type::CALC: 
    case ex_command::type::EXEC: 
    case ex_command::type::FIND_MARGIN: 
      SetValue(TCI().Get()); 
      SelectAll();
      break;

    case ex_command::type::COMMAND:
      if (command == ":!")
      {
        SetValue("!");
        SetInsertionPointEnd();
      }
      else if (!TCI().Get().empty())
      {
        SetValue(m_ModeVisual && TCI().Get().find(range) != 0 ? 
          range + TCI().Get(): TCI().Get()); 
        SelectAll();
      }
      else
      {
        SetValue(range); 
        SelectAll();
      }
      break;

    case ex_command::type::FIND: 
      SetValue(!m_ModeVisual ? ex->GetSTC()->GetFindString(): std::string()); 
      SelectAll();
      break;

    default:
      return false;
  }
    
  Show();
  SetFocus();

  return true;
}
