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
    auto* ex() {return m_ex;};
      
    // Sets ex component.
    // Returns false if command not supported.
    bool set_ex(wex::ex* ex, const std::string& command);
  private:
    textctrl_input& TCI() {
      switch (m_Command.type())
      {
        case ex_command::type_t::CALC: return m_Calcs;
        case ex_command::type_t::EXEC: return m_Execs;
        case ex_command::type_t::FIND_MARGIN: return m_FindMargins;
        default: return m_Commands;
      }};
    ex_command m_Command;
    managed_frame* m_Frame;
    wex::ex* m_ex {nullptr};
    wxStaticText* m_Prefix;
    bool m_ControlR {false}, m_ModeVisual {false}, m_UserInput {false};
    textctrl_input 
      m_Calcs {ex_command::type_t::CALC},
      m_Commands {ex_command::type_t::COMMAND},
      m_Execs {ex_command::type_t::EXEC},
      m_FindMargins {ex_command::type_t::FIND_MARGIN};
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
  add_toolBarPane(m_ToolBar, "TOOLBAR", _("Toolbar"));
  add_toolBarPane(m_FindBar,  "FINDBAR", _("Findbar"));
  add_toolBarPane(m_OptionsBar, "OPTIONSBAR", _("Optionsbar"));
  add_toolBarPane(CreateExPanel(), "VIBAR");
  m_Manager.Update();
  
  Bind(wxEVT_AUI_PANE_CLOSE, [=](wxAuiManagerEvent& event) {
    wxAuiPaneInfo* info = event.GetPane();
    info->BestSize(info->window->GetSize()).Fixed().Resizable();
    m_Manager.Update();
    // If this pane is a toolbar pane, it might have a checkbox,
    // update that as well.
    m_OptionsBar->update(info->name, false);
    });
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_FileHistory.save();
    event.Skip();});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    DoRecent(m_FileHistory, event.GetId() - m_FileHistory.get_base_id());},
    m_FileHistory.get_base_id(), 
    m_FileHistory.get_base_id() + m_FileHistory.get_max_files());

  for (const auto& it : m_ToggledPanes)
  {
    Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
      event.Check(m_Manager.GetPane(it.first.first).IsShown());}, it.second);
    Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
      toggle_pane(it.first.first);}, it.second);
  }
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    find_replace_data::get()->set_find_strings(std::list < std::string > {});}, ID_CLEAR_FINDS);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    stc::config_dialog(
      window_data().
        id(wxID_PREFERENCES).
        parent(this).
        title(_("Editor Options").ToStdString()).
        button(wxAPPLY | wxOK | wxCANCEL));}, wxID_PREFERENCES);
      
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_FileHistory.clear();}, ID_CLEAR_FILES);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (auto* stc = get_stc(); stc != nullptr)
    {
      auto it = find_replace_data::get()->get_find_strings().begin();
      std::advance(it, event.GetId() - ID_FIND_FIRST);
      if (const std::string text(*it); stc->find_next(text, 
        stc->get_vi().is_active()? stc->get_vi().search_flags(): -1))
      {
        find_replace_data::get()->set_find_string(text);
      }
    }}, ID_FIND_FIRST, ID_FIND_LAST);
}

wex::managed_frame::~managed_frame()
{
  delete m_Debug;
  
  m_Manager.UnInit();
}

bool wex::managed_frame::add_toolBarPane(
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

bool wex::managed_frame::allow_close(wxWindowID id, wxWindow* page)
{
  // The page will be closed, so do not update find focus now.
  set_find_focus(nullptr);
  return true;
}

void wex::managed_frame::append_panes(wxMenu* menu) const
{
#ifdef __WXMSW__
  menu->AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar\tCtrl+I"));
#endif

  menu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));

  for (const auto& it : m_ToggledPanes)
  {
    if (it.first.first == "PROCESS" && process::get_shell() == nullptr)
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
  m_TextCtrl = new textctrl(this, text, window_data().parent(panel));
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Center());
  sizer->Add(m_TextCtrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wex::managed_frame::DoRecent(
  const class file_history& history, 
  size_t index, 
  stc_data::window_t flags)
{
  if (const path file(history.get_history_file(index)); !file.data().empty())
  {
    open_file(file, stc_data().flags(flags));
  }
}

bool wex::managed_frame::show_ex_command(ex* ex, const std::string& command)
{
  return show_pane("VIBAR") && m_TextCtrl->set_ex(ex, command);
}

void wex::managed_frame::hide_ex_bar(int hide)
{
  if (m_Manager.GetPane("VIBAR").IsShown())
  {
    if (hide == HIDE_BAR_FORCE || hide == HIDE_BAR_FORCE_FOCUS_STC ||
        (GetStatusBar() != nullptr && GetStatusBar()->IsShown()))
    {
      show_pane("VIBAR", false);
    }
    
    if ((hide == HIDE_BAR_FOCUS_STC || hide == HIDE_BAR_FORCE_FOCUS_STC) && 
         m_TextCtrl != nullptr && 
         m_TextCtrl->ex() != nullptr)
    {
      m_TextCtrl->ex()->stc()->SetFocus();
    }
  }
}
  
void wex::managed_frame::on_notebook(wxWindowID id, wxWindow* page)
{
  if (auto* stc = wxDynamicCast(page, wex::stc); stc != nullptr)
  {
    set_recent_file(stc->get_filename());
  }
}

wex::stc* wex::managed_frame::open_file(
  const path& file,
  const stc_data& data)
{
  if (auto* stc = frame::open_file(file, data); stc != nullptr)
  {
    set_recent_file(file);
    return stc;
  }

  return nullptr;
}

void wex::managed_frame::print_ex(ex* ex, const std::string& text)
{
  ex->print(text);
}

void wex::managed_frame::set_recent_file(const path& path) 
{
  m_FileHistory.add(path);
}

void wex::managed_frame::show_ex_message(const std::string& text)
{
  if (GetStatusBar() != nullptr && GetStatusBar()->IsShown())
  {
    hide_ex_bar();
    GetStatusBar()->SetStatusText(text);
  }
  else
  {
    m_TextCtrl->SetValue(text);
  }
}

bool wex::managed_frame::show_pane(const std::string& pane, bool show)
{
  if (wxAuiPaneInfo& info = m_Manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else 
  {
    show ? info.Show(): info.Hide();
    m_Manager.Update();
    m_OptionsBar->update(pane, show);
    return true;
  }
}
  
void wex::managed_frame::sync_all()
{
  if (auto* stc = get_stc(); stc != nullptr)
  {
    stc->sync(config("AllowSync").get(true));
  }
}

void wex::managed_frame::sync_close_all(wxWindowID id)
{
  set_find_focus(nullptr);
}

// Implementation of support class.

wex::textctrl::textctrl(
  managed_frame* frame,
  wxStaticText* prefix,
  const window_data& data)
  : wxTextCtrl(
      data.parent(), 
      data.id(), 
      wxEmptyString, 
      data.pos(), 
      data.size(), 
      data.style() | wxTE_PROCESS_ENTER)
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
        m_Command.append(event.GetUnicodeKey());
      }
    }
        
    switch (event.GetKeyCode())
    {
      case WXK_CONTROL_R: 
        m_ControlR = true; 
        break;

      case WXK_TAB: {
        if (m_ex != nullptr && m_ex->stc()->get_filename().file_exists())
        {
          path::current(m_ex->stc()->get_filename().get_path());
        }

        // studio not yet: [[maybe_unused]]
        if (const auto& [r, e, v] = autocomplete_filename(m_Command.command());
          r)
        {
          m_Command.append(e);
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
              if (m_ex != nullptr) event.SetString(m_ex->stc()->get_filename().fullname());
            }
            else 
            {
              event.SetString(ex::get_macros().get_register(c));
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
        else if (m_Command.type() == ex_command::type_t::FIND)
        {
          find_replace_data::get()->m_FindStrings.set(event.GetKeyCode(), this); 
        }
        else
        {
          TCI().set(event.GetKeyCode(), this); 
        }
        break;
        
      case WXK_ESCAPE:
        if (m_ex != nullptr)
        {
          m_ex->stc()->position_restore();
        }
        m_Frame->hide_ex_bar(managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
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
      m_ex->stc()->position_save();
    }});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (m_UserInput && m_ex != nullptr && m_Command.type() == ex_command::type_t::FIND)
    {
      m_ex->stc()->position_restore();
      m_ex->stc()->find_next(
        GetValue().ToStdString(),
        m_ex->search_flags(),
        m_Prefix->GetLabel() == "/");
    }});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    if (m_ex == nullptr || GetValue().empty())
    {
      m_Frame->hide_ex_bar(managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
      return;
    }

    m_Command.command(m_Prefix->GetLabel().ToStdString() + GetValue().ToStdString());
    m_Command.is_handled(m_UserInput && m_Command.type() == ex_command::type_t::FIND);

    if (m_Command.exec())
    {
      int focus = (m_Command.type() == ex_command::type_t::FIND ? 
        managed_frame::HIDE_BAR_FORCE_FOCUS_STC: 
        managed_frame::HIDE_BAR_FOCUS_STC);

      if (m_Command.type() == ex_command::type_t::FIND)
      {
        find_replace_data::get()->set_find_string(GetValue().ToStdString());
      }
      else
      {
        TCI().set(this); 

        if (m_Command.type() == ex_command::type_t::COMMAND)
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

      m_Frame->hide_ex_bar(focus);
    }});
}

bool wex::textctrl::set_ex(wex::ex* ex, const std::string& command) 
{
  if (command.empty()) return false;

  m_ex = ex;
  m_UserInput = false;
  const std::string range(command.substr(1));
  m_ModeVisual = !range.empty();
  m_Prefix->SetLabel(command.substr(0, 1));
  m_Command = ex_command(ex->get_command()).command(command);
  m_ControlR = false;

  switch (m_Command.type())
  {
    case ex_command::type_t::CALC: 
    case ex_command::type_t::EXEC: 
    case ex_command::type_t::FIND_MARGIN: 
      SetValue(TCI().get()); 
      SelectAll();
      break;

    case ex_command::type_t::COMMAND:
      if (command == ":!")
      {
        SetValue("!");
        SetInsertionPointEnd();
      }
      else if (!TCI().get().empty())
      {
        SetValue(m_ModeVisual && TCI().get().find(range) != 0 ? 
          range + TCI().get(): TCI().get()); 
        SelectAll();
      }
      else
      {
        SetValue(range); 
        SelectAll();
      }
      break;

    case ex_command::type_t::FIND: 
      SetValue(!m_ModeVisual ? ex->stc()->get_find_string(): std::string()); 
      SelectAll();
      break;

    default:
      return false;
  }
    
  Show();
  SetFocus();

  return true;
}
