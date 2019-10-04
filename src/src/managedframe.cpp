////////////////////////////////////////////////////////////////////////////////
// Name:      managed_frame.cpp
// Purpose:   Implementation of wex::managed_frame class.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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

const auto ID_REGISTER = wxWindow::NewControlId();

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
      
    // Virtual interface.
    void AppendText(const wxString& text) override;
    void ChangeValue(const wxString& value) override;
    void Clear() override;
    void SetValue(const wxString& value) override;
    
    // Returns ex component.
    auto* ex() {return m_ex;};
      
    // Sets ex component.
    // Returns false if command not supported.
    bool set_ex(wex::ex* ex, const std::string& command);
  private:
    textctrl_input& TCI() {
      switch (m_command.type())
      {
        case ex_command::type_t::CALC: return m_calcs;
        case ex_command::type_t::EXEC: return m_execs;
        case ex_command::type_t::FIND_MARGIN: return m_find_margins;
        default: return m_commands;
      }};

    ex_command m_command;

    wex::ex* m_ex {nullptr};
    managed_frame* m_frame;
    wxStaticText* m_prefix;

    bool 
      m_control_r {false}, 
      m_control_r_present {false},
      m_mode_visual {false}, 
      m_user_input {false};

    textctrl_input 
      m_calcs {ex_command::type_t::CALC},
      m_commands {ex_command::type_t::COMMAND},
      m_execs {ex_command::type_t::EXEC},
      m_find_margins {ex_command::type_t::FIND_MARGIN};
  };
};

wex::managed_frame::managed_frame(size_t maxFiles, const window_data& data)
  : frame(data)
  , m_debug(new debug(this))
  , m_file_history(maxFiles, wxID_FILE1)
  , m_findbar(new find_toolbar(this))
  , m_optionsbar(new options_toolbar(this))
  , m_toolbar(new toolbar(this))
  , m_toggled_panes({
    {{"FINDBAR", _("&Findbar").ToStdString()}, ID_VIEW_LOWEST + 1},
    {{"OPTIONSBAR", _("&Optionsbar").ToStdString()}, ID_VIEW_LOWEST + 2},
    {{"TOOLBAR", _("&Toolbar").ToStdString()}, ID_VIEW_LOWEST + 3},
    {{"PROCESS", _("&Process").ToStdString()}, ID_VIEW_LOWEST + 4}})
{
  m_manager.SetManagedWindow(this);
  add_toolbar_pane(m_toolbar, "TOOLBAR", _("Toolbar"));
  add_toolbar_pane(m_findbar,  "FINDBAR", _("Findbar"));
  add_toolbar_pane(m_optionsbar, "OPTIONSBAR", _("Optionsbar"));
  add_toolbar_pane(create_ex_panel(), "VIBAR");
  m_manager.Update();
  
  Bind(wxEVT_AUI_PANE_CLOSE, [=](wxAuiManagerEvent& event) {
    wxAuiPaneInfo* info = event.GetPane();
    info->BestSize(info->window->GetSize()).Fixed().Resizable();
    m_manager.Update();
    // If this pane is a toolbar pane, it might have a checkbox,
    // update that as well.
    m_optionsbar->update(info->name, false);
    });
  
  Bind(wxEVT_CLOSE_WINDOW, [=](wxCloseEvent& event) {
    m_file_history.save();
    event.Skip();});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    on_menu_history(m_file_history, event.GetId() - m_file_history.get_base_id());},
    m_file_history.get_base_id(), 
    m_file_history.get_base_id() + m_file_history.get_max_files());

  for (const auto& it : m_toggled_panes)
  {
    Bind(wxEVT_UPDATE_UI, [=](wxUpdateUIEvent& event) {
      event.Check(m_manager.GetPane(it.first.first).IsShown());}, it.second);
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
    m_file_history.clear();}, ID_CLEAR_FILES);
    
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
  delete m_debug;
  
  m_manager.UnInit();
}

bool wex::managed_frame::add_toolbar_pane(
  wxWindow* window, 
  const std::string& name,
  const std::string& caption)
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
  
  return m_manager.AddPane(window, pane);
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
  menu->AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar\tCtrl+M"));
#endif

  menu->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));

  for (const auto& it : m_toggled_panes)
  {
    if (it.first.first == "PROCESS" && process::get_shell() == nullptr)
    {
      continue;
    }
    
    menu->AppendCheckItem(it.second, it.first.second);
  }
}
  
wxPanel* wex::managed_frame::create_ex_panel()
{
  // An ex panel starts with small static text for : or /, then
  // comes the ex ctrl for getting user input.
  wxPanel* panel = new wxPanel(this);
  wxStaticText* text = new wxStaticText(panel, wxID_ANY, " ");
  m_textctrl = new textctrl(this, text, window_data().parent(panel));
  
  wxFlexGridSizer* sizer = new wxFlexGridSizer(2);
  sizer->AddGrowableCol(1);
  sizer->Add(text, wxSizerFlags().Center());
  sizer->Add(m_textctrl, wxSizerFlags().Expand());
  
  panel->SetSizerAndFit(sizer);

  return panel;
}

void wex::managed_frame::on_menu_history(
  const class file_history& history, 
  size_t index, 
  stc_data::window_t flags)
{
  if (const path file(history.get_history_file(index)); !file.empty())
  {
    open_file(file, stc_data().flags(flags));
  }
}

bool wex::managed_frame::show_ex_command(ex* ex, const std::string& command)
{
  return show_pane("VIBAR") && m_textctrl->set_ex(ex, command);
}

void wex::managed_frame::hide_ex_bar(int hide)
{
  if (m_manager.GetPane("VIBAR").IsShown())
  {
    if (hide == HIDE_BAR_FORCE || hide == HIDE_BAR_FORCE_FOCUS_STC ||
        (GetStatusBar() != nullptr && GetStatusBar()->IsShown()))
    {
      show_pane("VIBAR", false);
    }
    
    if ((hide == HIDE_BAR_FOCUS_STC || hide == HIDE_BAR_FORCE_FOCUS_STC) && 
         m_textctrl != nullptr && 
         m_textctrl->ex() != nullptr)
    {
      m_textctrl->ex()->get_stc()->SetFocus();
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
  m_file_history.add(path);
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
    m_textctrl->SetValue(text);
  }
}

bool wex::managed_frame::show_pane(const std::string& pane, bool show)
{
  if (wxAuiPaneInfo& info = m_manager.GetPane(pane); !info.IsOk())
  {
    return false;
  }
  else 
  {
    show ? info.Show(): info.Hide();
    m_manager.Update();
    m_optionsbar->update(pane, show);
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
  , m_frame(frame)
  , m_prefix(prefix)
{
  SetFont(config(_("Text font")).get(
    wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (event.GetUnicodeKey() != WXK_NONE &&
        event.GetKeyCode() != WXK_TAB && 
        event.GetKeyCode() != WXK_RETURN)
    {
      m_command.append(event.GetUnicodeKey());
    }
        
    switch (event.GetKeyCode())
    {
      case WXK_TAB: {
        if (m_ex != nullptr && m_ex->get_stc()->get_filename().file_exists())
        {
          path::current(m_ex->get_stc()->get_filename().get_path());
        }

        // studio not yet: [[maybe_unused]]
        if (const auto& [r, e, v] = autocomplete_filename(m_command.command());
          r)
        {
          AppendText(e);
        }}
        break;

      default: {
        bool skip = true;

        if (event.GetKeyCode() != WXK_RETURN)
        {
          if (event.GetUnicodeKey() != (wxChar)WXK_NONE && m_control_r)
          {
            skip = false;
            const char c = event.GetUnicodeKey();
            wxCommandEvent event(wxEVT_MENU, ID_REGISTER);

            if (c == '%')
            {
              if (m_ex != nullptr) 
              {
                event.SetString(m_ex->get_stc()->get_filename().fullname());
              }
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

          m_user_input = true;
        }

        m_control_r = false;

        if (skip)
        {
          event.Skip();
        }
      }}});

  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    if (event.GetKeyCode() != WXK_RETURN && !GetStringSelection().empty())
    {
      // TODO: only clear selection
      m_command.set(m_prefix->GetLabel());
    }

    switch (event.GetKeyCode())
    {
      case 'r': 
      case 'R':
        if (event.ControlDown())
        {
          m_command.append(WXK_CONTROL_R);
          m_control_r = true; 
          m_control_r_present = true; 
        }
        else
        {
          event.Skip();
        }
        break;

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
        else if (m_command.type() == ex_command::type_t::FIND)
        {
          find_replace_data::get()->m_findStrings.set(event.GetKeyCode(), this); 
        }
        else
        {
          TCI().set(event.GetKeyCode(), this); 
        }
        break;
        
      case WXK_BACK:
        if (!m_command.empty()) 
        {
          m_command.pop_back();
        }
        event.Skip();
        break;

      case WXK_ESCAPE:
        if (m_ex != nullptr)
        {
          m_ex->get_stc()->position_restore();
        }
        m_frame->hide_ex_bar(managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
        m_control_r = false;
        m_user_input = false;
        break;

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
      m_ex->get_stc()->position_save();
    }});

  Bind(wxEVT_TEXT, [=](wxCommandEvent& event) {
    event.Skip();
    if (m_user_input && m_ex != nullptr && m_command.type() == ex_command::type_t::FIND)
    {
      m_ex->get_stc()->position_restore();
      m_ex->get_stc()->find_next(
        GetValue().ToStdString(),
        m_ex->search_flags(),
        m_prefix->GetLabel() == "/");
    }});

  Bind(wxEVT_TEXT_ENTER, [=](wxCommandEvent& event) {
    if (m_ex == nullptr || GetValue().empty())
    {
      m_frame->hide_ex_bar(managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
      return;
    }
      
    if (!m_control_r_present)
    {
      m_command.set(m_prefix->GetLabel() + GetValue());
    }

    if ((m_user_input && m_command.type() == ex_command::type_t::FIND)
      || m_command.exec())
    {
      int focus = (m_command.type() == ex_command::type_t::FIND ? 
        managed_frame::HIDE_BAR_FORCE_FOCUS_STC: 
        managed_frame::HIDE_BAR_FOCUS_STC);

      if (m_command.type() == ex_command::type_t::FIND)
      {
        find_replace_data::get()->set_find_string(GetValue().ToStdString());
      }
      else
      {
        TCI().set(this); 

        if (m_command.type() == ex_command::type_t::COMMAND)
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

      m_frame->hide_ex_bar(focus);
    }});
}

bool wex::textctrl::set_ex(wex::ex* ex, const std::string& command) 
{
  if (command.empty()) return false;

  m_ex = ex;
  m_user_input = false;
  const std::string range(command.substr(1));
  m_command = ex_command(ex->get_command()).set(command);
  m_mode_visual = !range.empty();
  m_prefix->SetLabel(command.substr(0, 1));
  m_control_r = false;
  m_control_r_present = false;

  switch (m_command.type())
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
        SetValue(m_mode_visual && TCI().get().find(range) != 0 ? 
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
      SetValue(!m_mode_visual ? ex->get_stc()->get_find_string(): std::string()); 
      SelectAll();
      break;

    default:
      return false;
  }
    
  Show();
  SetFocus();

  return true;
}

void wex::textctrl::AppendText(const wxString& text)
{
  m_command.append(text);
  wxTextCtrl::AppendText(text);
}

void wex::textctrl::ChangeValue(const wxString& value)
{
  m_command.set(m_prefix->GetLabel() + value);
  wxTextCtrl::ChangeValue(value);
}

void wex::textctrl::Clear()
{
  m_command.set(m_prefix->GetLabel());
  wxTextCtrl::Clear();
}
  
void wex::textctrl::SetValue(const wxString& value)
{
  m_command.set(m_prefix->GetLabel() + value);
  wxTextCtrl::SetValue(value);
}
