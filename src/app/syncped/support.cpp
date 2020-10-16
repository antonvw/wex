////////////////////////////////////////////////////////////////////////////////
// Name:      support.cpp
// Purpose:   Implementation of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/beautify.h>
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/file-dialog.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/menu.h>
#include <wex/menubar.h>
#include <wex/report/listview-file.h>
#include <wex/shell.h>
#include <wex/statusbar.h>
#include <wex/stc.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/version.h>
#include <wx/aboutdlg.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#ifndef __WXMSW__
#include "app.xpm"
#endif
#include "app.h"
#include "defs.h"
#include "support.h"

const long pane_flag = wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_CLOSE_ON_ALL_TABS |
                       wxAUI_NB_CLOSE_BUTTON | wxAUI_NB_WINDOWLIST_BUTTON |
                       wxAUI_NB_SCROLL_BUTTONS;

void build_ascii_table(wex::frame* frame)
{
  auto* stc = frame->open_file(wex::path("Ascii table"), std::string());

  // Do not show an edge, eol whitespace for ascii table.
  stc->SetEdgeMode(wxSTC_EDGE_NONE);
  stc->SetViewEOL(false);
  stc->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
  stc->SetTabWidth(5);

  for (int i = 1; i <= 255; i++)
  {
    switch (i)
    {
      case 9:
        stc->add_text(wxString::Format("%3d\tTAB", i));
        break;
      case 10:
        stc->add_text(wxString::Format("%3d\tLF", i));
        break;
      case 13:
        stc->add_text(wxString::Format("%3d\tCR", i));
        break;
      default:
        stc->add_text(wxString::Format("%3d\t%c", i, (wxUniChar)i));
    }
    stc->add_text((i % 5 == 0) ? stc->eol() : "\t");
  }

  stc->EmptyUndoBuffer();
  stc->SetSavePoint();
  stc->SetReadOnly(true);
}

decorated_frame::decorated_frame(app* app)
  : wex::report::frame(
      25, // maxFiles
      4,  // maxProjects
      wex::data::window().name("mainFrame").style(wxDEFAULT_FRAME_STYLE))
  , m_app(app)
  , m_dirctrl(new wex::report::dirctrl(this))
  , m_editors(
      new editors(wex::data::window().id(ID_NOTEBOOK_EDITORS).style(pane_flag)))
  , m_lists(new wex::notebook(
      wex::data::window().id(ID_NOTEBOOK_LISTS).style(pane_flag)))
  , m_process(new wex::process())
  , m_projects(new wex::notebook(
      wex::data::window().id(ID_NOTEBOOK_PROJECTS).style(pane_flag)))
{
  SetIcon(wxICON(app));

  wex::process::prepare_output(this);

  pane_add(
    {{m_editors,
      wxAuiPaneInfo().CenterPane().MaximizeButton(true).Name("FILES").Caption(
        _("Files"))},
     {m_dirctrl,
      wxAuiPaneInfo()
        .Left()
        .Hide()
        .MaximizeButton(true)
        .CloseButton(false)
        .Name("DIRCTRL")
        .MinSize(200, 150)
        .Caption(_("Explorer"))},
     {m_lists,
      wxAuiPaneInfo()
        .Bottom()
        .Hide()
        .MaximizeButton(true)
        .MinSize(250, 100)
        .Name("OUTPUT")
        .Row(0)
        .Caption(_("Output"))},
     {m_process->get_shell(),
      wxAuiPaneInfo().Bottom().Hide().Name("PROCESS").MinSize(250, 100).Caption(
        _("Process"))},
     {m_projects,
      wxAuiPaneInfo()
        .Left()
        .Hide()
        .MaximizeButton(true)
        .Name("PROJECTS")
        .MinSize(150, 150)
        .Caption(_("Projects"))}});

  if (wex::config("show.History").get(false))
  {
    add_pane_history();
  }

#ifdef __WXMSW__
  const int lexer_size = 60;
#else
  const int lexer_size = 75;
#endif
  setup_statusbar(
    {{"PaneFileType", 50},
     {"PaneInfo", 100},
     {"PaneLexer", lexer_size},
     {"PaneTheme", lexer_size},
     {"PaneVCS", -2},
     {"PaneDBG", 50, false},
     {"PaneMacro", -1, false},
     {"PaneMode", 100, false}});

  if (wex::vcs vcs; vcs.use() && wex::vcs::size() > 0)
  {
    const auto b(vcs.set_entry_from_base() ? vcs.get_branch() : std::string());
    statustext(!b.empty() ? b : vcs.name(), "PaneVCS");
  }
  else
  {
    m_statusbar->pane_show("PaneVCS", false);
  }

  if (wex::lexers::get()->get_lexers().empty())
  {
    m_statusbar->pane_show("PaneLexer", false);
    m_statusbar->pane_show("PaneTheme", false);
  }

  auto* menuFind = new wex::menu();

  if (wex::config(_("stc.vi mode")).get(false))
  {
    // No accelerators for vi mode, Ctrl F is page down.
    menuFind->append(
      {{wxID_FIND, wxGetStockLabel(wxID_FIND, wxSTOCK_NOFLAGS)}});

    if (m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY))
    {
      menuFind->append(
        {{wxID_REPLACE, wxGetStockLabel(wxID_REPLACE, wxSTOCK_NOFLAGS)}});
    }
  }
  else
  {
    menuFind->append({{wxID_FIND}});

    if (!m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY))
    {
      menuFind->append({{wxID_REPLACE}});
    }
  }

  menuFind->append(
    {{wex::ID_TOOL_REPORT_FIND, wex::ellipsed(_("Find &in Files"))}});

  if (!m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY))
  {
    menuFind->append(
      {{wex::ID_TOOL_REPLACE, wex::ellipsed(_("Replace in File&s"))}});
  }

  auto* menuMacro = new wex::menu(
    {{ID_EDIT_MACRO_START_RECORD, wex::ellipsed(_("Start Record"))},
     {ID_EDIT_MACRO_STOP_RECORD, _("Stop Record")},
     {},
     {ID_EDIT_MACRO_PLAYBACK, wex::ellipsed(_("Playback"))}});

  if (wex::ex::get_macros().get_filename().file_exists())
  {
    menuMacro->append(
      {{},
       {ID_EDIT_MACRO,
        wxGetStockLabel(wxID_EDIT),
        wex::data::menu().action([=](wxCommandEvent&) {
          open_file(wex::ex::get_macros().get_filename());
        })}});
  }

  wex::menu* menuDebug = nullptr;

  if (m_app->get_is_debug())
  {
    menuDebug = new wex::menu();

    if (get_debug()->add_menu(menuDebug) == 0)
    {
      delete menuDebug;
      menuDebug = nullptr;
      wex::log() << "no debug menu present";
    }
    else
    {
      statustext(get_debug()->debug_entry().name(), "PaneDBG");
    }

    m_statusbar->pane_show("PaneDBG", true);
  }

  auto* menuOptions = new wex::menu();

  if (wex::vcs::size() > 0)
  {
    menuOptions->append(
      {{wxWindow::NewControlId(),
        wex::ellipsed(_("Set &VCS")),
        wex::data::menu().action([=](wxCommandEvent&) {
          if (wex::vcs().config_dialog() == wxID_OK)
          {
            wex::vcs vcs;
            vcs.set_entry_from_base(this);
            m_statusbar->pane_show("PaneVCS", vcs.use());
            statustext(vcs.name(), "PaneVCS");
          }
        })},
       {}});
  }

  menuOptions->append({
#ifndef __WXOSX__
    {wxID_PREFERENCES, wex::ellipsed(_("Set &Editor Options"))},
#else
    {wxID_PREFERENCES},
#endif
    {ID_OPTION_LIST,
     wex::ellipsed(_("Set &List Options")),
     wex::data::menu().action([=](wxCommandEvent& event) {
       wex::listview::config_dialog(wex::data::window()
                                      .button(wxOK | wxCANCEL | wxAPPLY)
                                      .id(ID_OPTION_LIST));
     })},
    {ID_OPTION_TAB,
     wex::ellipsed(_("Set &Tab Options")),
     wex::data::menu().action([=](wxCommandEvent& event) {
       wex::notebook::config_dialog(wex::data::window()
                                      .button(wxOK | wxCANCEL | wxAPPLY)
                                      .id(ID_OPTION_TAB));
     })}});

  SetMenuBar(new wex::menubar(
    {{new wex::menu(
        {{wxID_NEW,
          wex::ellipsed(wxGetStockLabel(wxID_NEW, wxSTOCK_NOFLAGS), "\tCtrl+N"),
          wex::data::menu().action([=](wxCommandEvent& event) {
            // In hex mode we cannot edit the file.
            if (wex::config("is_hexmode").get(false))
              return;

            static std::string name;

            if (event.GetString().empty())
            {
              wxTextEntryDialog dlg(
                this,
                _("Input") + ":",
                _("File Name"),
                name);
              wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
              validator.SetCharExcludes("/\\?%*:|\"<>");
              dlg.SetTextValidator(validator);

              if (dlg.ShowModal() == wxID_CANCEL)
                return;

              name = dlg.GetValue();
            }
            else
            {
              name = event.GetString();
            }

            if (!name.empty())
            {
              auto* page = new wex::stc(
                std::string(),
                wex::data::stc(m_app->data())
                  .window(wex::data::window().parent(m_editors)));
              page->get_file().file_new(name);
              // This file does yet exist, so do not give it a bitmap.
              m_editors->add_page(
                wex::data::notebook().page(page).key(name).select());
              pane_show("FILES");
            };
          })},

         {wxID_OPEN},

         {},

         {wxWindow::NewControlId(),
          file_history(),
          wex::data::menu().ui([=](wxUpdateUIEvent& event) {
            event.Enable(!file_history().get_history_file().empty());
          })},

         {wxID_CLOSE,
          wxGetStockLabel(wxID_CLOSE, wxSTOCK_NOFLAGS) + "\tCtrl+W",
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              if (auto* stc = get_stc(); stc != nullptr)
              {
                if (!allow_close(m_editors->GetId(), stc))
                  return;
                m_editors->delete_page(m_editors->key_by_page(stc));
              };
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Enable(
                m_editors->IsShown() && m_editors->GetPageCount() > 0);
            })},

         {},

         {wxID_SAVE},

         {wxID_SAVEAS, "", wex::data::menu().ui([=](wxUpdateUIEvent& event) {
            event.Enable(m_editors->IsShown() && m_editors->GetPageCount() > 0);
          })},

         {wex::ID_ALL_SAVE,
          _("Save A&ll"),
          wex::data::menu().art(wxART_FILE_SAVE)},

         {},

         {wex::menu_item::PRINT},

         {},

         {wex::menu_item::EXIT}}),

      wxGetStockLabel(wxID_FILE)},

     {new wex::menu(
        {{wxID_UNDO},
         {wxID_REDO},
         {},
         {wxID_CUT},
         {wxID_COPY},
         {wxID_PASTE},
         {},
         {wxID_JUMP_TO},
         {wxID_CLEAR},
         {wxID_SELECTALL},
         {},
         {menuFind,
          !m_app->data().flags().test(wex::data::stc::WIN_READ_ONLY) ?
            _("&Find and Replace") :
            _("&Find")},
         {},
         {wex::ID_EDIT_CONTROL_CHAR,
          wex::ellipsed(_("&Control Char"), "Ctrl+K"),
          wex::data::menu().action([=](wxCommandEvent& event) {
            if (get_stc() != nullptr)
            {
              wxPostEvent(get_stc(), event);
            }
          })},
         {},
         {menuMacro, _("&Macro"), ID_EDIT_MACRO_MENU}}),
      wxGetStockLabel(wxID_EDIT)},

     {new wex::menu(
        {{this},
         {},
         {wxWindow::NewControlId(),
          _("&Files"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("FILES");
              if (!pane_is_shown("FILES") && pane_is_shown("PROJECTS"))
              {
                pane_maximize("PROJECTS");
              };
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("FILES"));
            })},

         {wxWindow::NewControlId(),
          _("&Projects"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("PROJECTS");
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("PROJECTS"));
            })},

         {ID_VIEW_DIRCTRL,
          _("&Explorer"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("DIRCTRL");
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("DIRCTRL"));
            })},

         {ID_VIEW_HISTORY,
          _("&History"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              if (m_history == nullptr)
              {
                add_pane_history();
              }
              else
              {
                pane_toggle("HISTORY");
              };
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(m_history != nullptr && pane_is_shown("HISTORY"));
            })},

         {wxWindow::NewControlId(),
          _("&Output"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              pane_toggle("OUTPUT");
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(pane_is_shown("OUTPUT"));
            })},

         {},

         {wxWindow::NewControlId(),
          _("&Ascii Table"),
          wex::data::menu().action([=](wxCommandEvent& event) {
            build_ascii_table(this);
          })}}),
      _("&View")},

     {new wex::menu(
        {{wxWindow::NewControlId(),
          wex::ellipsed(_("&Select")),
          wex::data::menu().action([=](wxCommandEvent& event) {
            if (wex::process::config_dialog() == wxID_OK)
            {
              pane_show("PROCESS");
              m_process->execute();
            };
          })},

         {},

         {wxID_EXECUTE,
          "",
          wex::data::menu().action([=](wxCommandEvent& event) {
            pane_show("PROCESS");
            m_process->execute();
          })},

         {wxID_STOP, "", wex::data::menu().action([=](wxCommandEvent& event) {
            m_process->stop();
            pane_show("PROCESS", false);
          })}}),
      _("&Process")},

     {new wex::menu(
        {{wxWindow::NewControlId(),
          wxGetStockLabel(wxID_NEW),
          wex::data::menu().art(wxART_NEW).action([=](wxCommandEvent& event) {
            const std::string text =
              wxString::Format("%s%d", _("project"), m_project_id++)
                .ToStdString();
            const wex::path fn(
              (!get_project_history().get_history_file().empty() ?
                 get_project_history().get_history_file().get_path() :
                 wex::config::dir()),
              text + ".prj");
            wxWindow* page = new wex::report::file(
              fn.string(),
              wex::data::window().parent(m_projects));
            ((wex::report::file*)page)->file_new(fn.string());
            // This file does yet exist, so do not give it a bitmap.
            m_projects->add_page(wex::data::notebook()
                                   .page(page)
                                   .key(fn.string())
                                   .caption(text)
                                   .select());
            set_recent_project(fn.string());
            pane_show("PROJECTS");
          })},

         {wxWindow::NewControlId(),
          wxGetStockLabel(wxID_OPEN),
          wex::data::menu()
            .art(wxART_FILE_OPEN)
            .action([=](wxCommandEvent& event) {
              wxFileDialog dlg(
                this,
                _("Select Projects"),
                (!get_project_history().get_history_file().empty() ?
                   get_project_history().get_history_file().get_path() :
                   wex::config::dir()),
                wxEmptyString,
                m_project_wildcard,
// osx asserts on GetPath with wxFD_MULTIPLE flag,
// and the to_vector_path does not do anything
#ifdef __WXOSX__
                wxFD_OPEN);
#else
                wxFD_OPEN | wxFD_MULTIPLE);
#endif
              if (dlg.ShowModal() == wxID_CANCEL)
                return;
              const std::vector<wex::path> v(
#ifdef __WXOSX__
                {dlg.GetPath().ToStdString()});
#else
                wex::to_vector_path(dlg).get());
#endif
              wex::open_files(
                this,
                v,
                wex::data::stc().flags(wex::data::stc::window_t().set(
                  wex::data::stc::WIN_IS_PROJECT)));
            })},

         {wxWindow::NewControlId(),
          _("&Open as Text"),
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              if (auto* project = get_project(); project != nullptr)
              {
                if (
                  wex::file_dialog(project).show_modal_if_changed() !=
                  wxID_CANCEL)
                {
                  open_file(project->get_filename());
                }
              };
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Enable(
                get_project() != nullptr &&
                !get_project()->get_filename().empty());
            })},

         {},

         {wxWindow::NewControlId(),
          get_project_history(),
          wex::data::menu().ui([=](wxUpdateUIEvent& event) {
            event.Enable(!get_project_history().get_history_file().empty());
          })},

         {wxWindow::NewControlId(),
          wxGetStockLabel(wxID_CLOSE),
          wex::data::menu()
            .art(wxART_CLOSE)
            .action([=](wxCommandEvent& event) {
              if (auto* project = get_project(); project != nullptr)
              {
                m_projects->delete_page(project->get_filename().string());
              };
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Enable(
                get_project() != nullptr && get_project()->IsShown());
            })},

         {wex::report::ID_PROJECT_SAVE,
          wxGetStockLabel(wxID_SAVE),
          wex::data::menu().art(wxART_FILE_SAVE)},

         {wxWindow::NewControlId(),
          wxGetStockLabel(wxID_SAVEAS),
          wex::data::menu()
            .art(wxART_FILE_SAVE_AS)
            .action([=](wxCommandEvent& event) {
              if (auto* project = get_project(); project != nullptr)
              {
                wex::file_dialog dlg(
                  project,
                  wex::data::window()
                    .style(wxFD_SAVE)
                    .parent(this)
                    .title(_("Project Save As").ToStdString())
                    .wildcard(m_project_wildcard));
                if (dlg.ShowModal() == wxID_OK)
                {
                  project->file_save(dlg.GetPath().ToStdString());
                  m_projects->set_page_text(
                    m_projects->key_by_page(project),
                    project->get_filename().string(),
                    project->get_filename().name());
                }
              };
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Enable(
                get_project() != nullptr && get_project()->IsShown());
            })},

         {},

         {wxWindow::NewControlId(),
          _("&Auto Sort"),
          wex::menu_item::CHECK,
          wex::data::menu()
            .action([=](wxCommandEvent& event) {
              wex::config("list.SortSync")
                .set(!wex::config("list.SortSync").get(true));
            })
            .ui([=](wxUpdateUIEvent& event) {
              event.Check(wex::config("list.SortSync").get(true));
            })}}),
      _("&Project")},

     {menuDebug, _("&Debug")},

     {menuOptions, _("&Options")},

     {new wex::menu(
        {{wxID_ABOUT, "", wex::data::menu().action([=](wxCommandEvent& event) {
            wxAboutDialogInfo info;
            info.SetIcon(GetIcon());
            info.SetVersion(wex::get_version_info().get());
            wxString description(_("This program offers a portable text or "
                                   "binary editor\n"
                                   "with automatic syncing."));
#ifdef __WXMSW__
            description += _(" All its config files are read\n"
                             "and saved in the same directory as "
                             "where the executable is.");
#endif
            description += "\n\nUsing:\n" +
                           wex::get_version_info().external_libraries().str();

            info.SetDescription(description);
            info.SetCopyright(wex::get_version_info().copyright());
            info.SetWebSite("http://sourceforge.net/projects/syncped/");
            wxAboutBox(info, this);
          })},

         {wxID_HELP, "", wex::data::menu().action([=](wxCommandEvent& event) {
            wxLaunchDefaultBrowser(
              "http://antonvw.github.io/syncped/v" +
              wex::get_version_info().get() + "/syncped.htm");
          })}}),
      wxGetStockLabel(wxID_HELP)}}));
}

void decorated_frame::add_pane_history()
{
  m_history = new wex::report::listview(
    wex::data::listview().type(wex::data::listview::HISTORY));

  pane_add(
    {{m_history,
      wxAuiPaneInfo()
        .Left()
        .MaximizeButton(true)
        .Name("HISTORY")
        .CloseButton(false)
        .MinSize(150, 150)
        .Caption(_("History"))}},
    std::string());
}

bool decorated_frame::allow_close(wxWindowID id, wxWindow* page)
{
  switch (id)
  {
    case ID_NOTEBOOK_EDITORS:
      if (auto* stc = (wex::stc*)page;
          wex::file_dialog(&stc->get_file()).show_modal_if_changed() ==
          wxID_CANCEL)
      {
        return false;
      }
      else if (wex::beautify b; b.is_active() && stc->get_file().is_written())
      {
        stc->get_file().close();
        b.file(stc->get_filename());
      }
      break;
    case ID_NOTEBOOK_PROJECTS:
      if (
        wex::file_dialog((wex::report::file*)page).show_modal_if_changed() ==
        wxID_CANCEL)
      {
        return false;
      }
      break;
  }

  return wex::report::frame::allow_close(id, page);
}

void decorated_frame::on_notebook(wxWindowID id, wxWindow* page)
{
  wex::report::frame::on_notebook(id, page);

  switch (id)
  {
    case ID_NOTEBOOK_EDITORS:
      ((wex::stc*)page)->properties_message();
      break;

    case ID_NOTEBOOK_LISTS:
      break;

    case ID_NOTEBOOK_PROJECTS:
      wex::log::status() << ((wex::report::file*)page)->get_filename();
      update_statusbar((wex::report::file*)page);
      break;

    default:
      assert(0);
  }
}

editors::editors(const wex::data::window& data)
  : wex::notebook(data)
{
  Bind(
    wxEVT_MENU,
    [=](wxCommandEvent& event) {
      wxPostEvent(wxAuiNotebook::GetCurrentPage(), event);
    },
    wex::ID_EDIT_VCS_LOWEST,
    wex::ID_EDIT_VCS_HIGHEST);

  Bind(wxEVT_AUINOTEBOOK_END_DRAG, [=](wxAuiNotebookEvent& event) {
    event.Skip();
    m_split = true;
  });

  Bind(wxEVT_AUINOTEBOOK_TAB_RIGHT_UP, [=](wxAuiNotebookEvent& event) {
    wex::menu menu(wex::menu::menu_t().set(wex::menu::IS_POPUP));

    auto* split = new wex::menu(
      {{ID_SPLIT_VERTICALLY, _("Split Vertically")},
       {ID_SPLIT_HORIZONTALLY, _("Split Horizontally")},
       {},
       {ID_SPLIT, _("Split")}});

    if (GetPageCount() > 1)
    {
      split->append(
        {{},
         {wxWindow::NewControlId(),
          _("Rearrange Vertically"),
          wex::data::menu().action([=](wxCommandEvent&) {
            rearrange(wxLEFT);
          })},
         {wxWindow::NewControlId(),
          _("Rearrange Horizontally"),
          wex::data::menu().action([=](wxCommandEvent&) {
            rearrange(wxTOP);
          })}});
    }

    menu.append(
      {{split, _("Split"), wxWindow::NewControlId()},
       {},
       {wxID_CLOSE},
       {wex::ID_ALL_CLOSE, _("Close A&ll")}});

    if (GetPageCount() > 2)
    {
      menu.append({{wex::ID_ALL_CLOSE_OTHERS, _("Close Others")}});
    }

    if (auto* stc = dynamic_cast<wex::stc*>(wxAuiNotebook::GetCurrentPage());
        stc->get_file().get_filename().file_exists() &&
        wex::vcs::dir_exists(stc->get_file().get_filename()))
    {
      menu.append({{}, {stc->get_file().get_filename()}});
    }

    PopupMenu(&menu);
  });
}
