////////////////////////////////////////////////////////////////////////////////
// Name:      bind.cpp
// Purpose:   Implementation of wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ctags/ctags.h>
#include <wex/del/frame.h>
#include <wex/del/listview-file.h>
#include <wex/factory/bind.h>
#include <wex/stc/stc.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/ui/frd.h>
#include <wex/vcs/debug.h>
#include <wex/vcs/vcs.h>

#define IN_FILES(ACTION, DIALOG)                                         \
  {                                                                      \
    if (!event.GetString().empty())                                      \
    {                                                                    \
      ACTION(event.GetString());                                         \
    }                                                                    \
    else                                                                 \
    {                                                                    \
      if (get_stc() != nullptr && !get_stc()->get_find_string().empty()) \
      {                                                                  \
        DIALOG->reload();                                                \
      }                                                                  \
      DIALOG->Show();                                                    \
    }                                                                    \
  }

#define TOOL_ACTION(TOOL, DIALOG, ACTION) \
  {[=, this](wxCommandEvent& event)       \
   {                                      \
     IN_FILES(ACTION, DIALOG)             \
   },                                     \
   TOOL},

void wex::del::frame::bind_all()
{
  Bind(
    wxEVT_CLOSE_WINDOW,
    [=, this](wxCloseEvent& event)
    {
      m_project_history.save();
      stc::on_exit();
      ctags::close();
      delete lexers::set(nullptr);
      delete printing::set(nullptr);

      config("show.MenuBar")
        .set(GetMenuBar() != nullptr && GetMenuBar()->IsShown());
      delete m_debug;

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
        open_from_event(event, std::string());
      },
      wxID_OPEN},

     {[=, this](wxCommandEvent& event)
      {
        stc::config_dialog(data::window()
                             .id(wxID_PREFERENCES)
                             .parent(this)
                             .title(_("Editor Options"))
                             .button(wxAPPLY | wxOK | wxCANCEL));
      },
      wxID_PREFERENCES},

     {[=, this](wxCommandEvent& event)
      {
        find_replace_data::get()->set_find_strings(config::strings_t{});
      },
      ID_CLEAR_FINDS},

     {[=, this](wxCommandEvent& event)
      {
        file_history().clear();
      },
      ID_CLEAR_FILES},

     {[=, this](wxCommandEvent& event)
      {
        m_project_history.clear();
      },
      ID_CLEAR_PROJECTS},

     {[=, this](wxCommandEvent& event)
      {
        if (auto* stc = dynamic_cast<wex::stc*>(get_stc()); stc != nullptr)
        {
          auto it = find_replace_data::get()->get_find_strings().begin();
          std::advance(it, event.GetId() - ID_FIND_FIRST);
          if (const std::string text(*it); stc->find(
                text,
                stc->get_vi().is_active() ? stc->get_vi().search_flags() : -1))
          {
            find_replace_data::get()->set_find_string(text);
          }
        }
      },
      ID_FIND_FIRST},

     {[=, this](wxCommandEvent& event)
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

     {[=, this](wxCommandEvent& event)
      {
        // this code handles the PaneVCS statusbar_clicked
        wex::vcs(
          std::vector<wex::path>(),
          event.GetId() - wex::ID_EDIT_VCS_LOWEST - 1)
          .request();
      },
      wex::ID_EDIT_VCS_LOWEST},

     {[=, this](wxCommandEvent& event)
      {
        SetMenuBar(GetMenuBar() != nullptr ? nullptr : m_menubar);
      },
      ID_VIEW_MENUBAR},

     {[=, this](wxCommandEvent& event)
      {
        SetWindowStyleFlag(
          !(GetWindowStyleFlag() & wxCAPTION) ?
            wxDEFAULT_FRAME_STYLE :
            GetWindowStyleFlag() & ~wxCAPTION);
        Refresh();
      },
      ID_VIEW_TITLEBAR}});
}
