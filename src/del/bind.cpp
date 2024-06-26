////////////////////////////////////////////////////////////////////////////////
// Name:      bind.cpp
// Purpose:   Implementation of wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/wex.h>

#define IN_FILES(ACTION, DIALOG)                                               \
  {                                                                            \
    if (!event.GetString().empty())                                            \
    {                                                                          \
      ACTION(event.GetString());                                               \
    }                                                                          \
    else                                                                       \
    {                                                                          \
      if (get_stc() != nullptr && !get_stc()->get_find_string().empty())       \
      {                                                                        \
        DIALOG->reload();                                                      \
      }                                                                        \
      DIALOG->Show();                                                          \
    }                                                                          \
  }

#define TOOL_ACTION(TOOL, DIALOG, ACTION)                                      \
  {[=, this](wxCommandEvent& event)                                            \
   {                                                                           \
     IN_FILES(ACTION, DIALOG)                                                  \
   },                                                                          \
   TOOL},

void wex::del::frame::bind_all()
{
  Bind(
    wxEVT_CLOSE_WINDOW,
    [=, this](wxCloseEvent& event)
    {
      m_project_history.save();

      config("show.MenuBar")
        .set(GetMenuBar() != nullptr && GetMenuBar()->IsShown());

      event.Skip();
    });

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event)
    {
      on_menu_history(
        m_project_history,
        event.GetId() - m_project_history.get_base_id(),
        wex::data::stc::window_t().set(data::stc::WIN_IS_PROJECT));
    },
    m_project_history.get_base_id(),
    m_project_history.get_base_id() + m_project_history.get_max_files());

  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        m_is_command = true;
        open_from_action(event.GetString(), std::string());
      },
      wxID_OPEN},

     {[=, this](const wxCommandEvent& event)
      {
        stc::config_dialog(data::window()
                             .id(wxID_PREFERENCES)
                             .parent(this)
                             .title(_("Editor Options"))
                             .button(wxAPPLY | wxOK | wxCANCEL));
      },
      wxID_PREFERENCES},

     {[=, this](const wxCommandEvent& event)
      {
        find_replace_data::get()->set_find_strings(config::strings_t{});
      },
      ID_CLEAR_FINDS},

     {[=, this](const wxCommandEvent& event)
      {
        file_history().clear();
      },
      ID_CLEAR_FILES},

     {[=, this](const wxCommandEvent& event)
      {
        m_project_history.clear();
      },
      ID_CLEAR_PROJECTS},

     {[=, this](const wxCommandEvent& event)
      {
        if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
        {
          auto it = find_replace_data::get()->get_find_strings().begin();
          std::advance(it, event.GetId() - ID_FIND_FIRST);
          if (const auto& text(*it); stc->find(
                text,
                stc->get_vi().is_active() ? stc->get_vi().search_flags() : -1))
          {
            find_replace_data::get()->set_find_string(text);
          }
        }
      },
      ID_FIND_FIRST},

     {[=, this](const wxCommandEvent& event)
      {
        if (auto* project = get_project(); project != nullptr)
        {
          project->file_save();
        }
      },
      ID_PROJECT_SAVE},

     // clang-format off
     TOOL_ACTION(ID_TOOL_REPLACE, m_rif_dialog, sed)
     TOOL_ACTION(ID_TOOL_REPORT_FIND, m_fif_dialog, grep)
     // clang-format on

     {[=, this](const wxCommandEvent& event)
      {
        // this code handles the PaneVCS statusbar_clicked
        wex::vcs(
          std::vector<wex::path>(),
          event.GetId() - wex::ID_EDIT_VCS_LOWEST - 1)
          .request();
      },
      wex::ID_EDIT_VCS_LOWEST},

     {[=, this](const wxCommandEvent& event)
      {
        SetMenuBar(GetMenuBar() != nullptr ? nullptr : m_menubar);
      },
      ID_VIEW_MENUBAR},

     {[=, this](const wxCommandEvent& event)
      {
        SetWindowStyleFlag(
          !(GetWindowStyleFlag() & wxCAPTION) ?
            wxDEFAULT_FRAME_STYLE :
            GetWindowStyleFlag() & ~wxCAPTION);
        Refresh();
      },
      ID_VIEW_TITLEBAR}});
}
