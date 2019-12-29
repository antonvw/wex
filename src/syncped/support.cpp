////////////////////////////////////////////////////////////////////////////////
// Name:      support.cpp
// Purpose:   Implementation of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/filedlg.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/menu.h>
#include <wex/menubar.h>
#include <wex/stc.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/version.h>
#include <wex/report/listviewfile.h>
#ifndef __WXMSW__
#include "app.xpm"
#endif
#include "support.h"
#include "app.h"
#include "defs.h"

decorated_frame::decorated_frame(app* app)
  : wex::report::frame(
      25,  // maxFiles
      4,   // maxProjects
      wex::window_data().name("mainFrame").style(wxDEFAULT_FRAME_STYLE))
  , m_app(app)
  , m_editors(new editors(
      wex::window_data().id(wex::ID_NOTEBOOK_EDITORS).style(m_pane_flag)))
  , m_lists(new wex::notebook(
      wex::window_data().id(wex::ID_NOTEBOOK_LISTS).style(m_pane_flag)))
  , m_process(new wex::process())
  , m_projects(new wex::notebook(
      wex::window_data().id(wex::ID_NOTEBOOK_PROJECTS).style(m_pane_flag)))
{
  SetIcon(wxICON(app));
  
  wex::process::prepare_output(this);

#ifdef __WXMSW__
  const int lexer_size = 60;
#else
  const int lexer_size = 75;
#endif
  setup_statusbar({
    {"PaneFileType", 50, _("File type").ToStdString()},
    {"PaneInfo", 100, _("Lines or items").ToStdString()},
    {"PaneLexer", lexer_size, _("Lexer").ToStdString()},
    {"PaneTheme", lexer_size, _("Theme").ToStdString()},
    {"PaneVCS", -2},
    {"PaneDBG", 50, "Debugger", wxSB_NORMAL, false},
    {"PaneMacro", -1, "macro", wxSB_NORMAL, false},
    {"PaneMode", 100, "vi mode", wxSB_NORMAL, false}});
  
  if (wex::vcs vcs; vcs.use() && wex::vcs::size() > 0)
  {
    const auto b(vcs.set_entry_from_base() ? vcs.get_branch(): std::string());
    statustext(!b.empty() ? b: vcs.name(), "PaneVCS");
  }
  else
  {
    m_statusbar->show_pane("PaneVCS", false);
  }
  
  if (wex::lexers::get()->get_lexers().empty())
  {
    m_statusbar->show_pane("PaneLexer", false);
    m_statusbar->show_pane("PaneTheme", false);
  }
  
  auto *menuEdit = new wex::menu({
    {wxID_UNDO}, {wxID_REDO}, {},
    {wxID_CUT}, {wxID_COPY}, {wxID_PASTE}, {},
    {wxID_JUMP_TO}, {wxID_CLEAR}, {wxID_SELECTALL}, {}});

  auto* menuFind = new wex::menu();
  
  if (wex::config(_("stc.vi mode")).get(false))
  {
    // No accelerators for vi mode, Ctrl F is page down.
    menuFind->append({{wxID_FIND, wxGetStockLabel(wxID_FIND, wxSTOCK_NOFLAGS)}});

    if (m_app->data().flags().test(wex::stc_data::WIN_READ_ONLY))
    {
      menuFind->append({{wxID_REPLACE, wxGetStockLabel(wxID_REPLACE, wxSTOCK_NOFLAGS)}});
    }
  }
  else
  {
    menuFind->append({{wxID_FIND}});

    if (!m_app->data().flags().test(wex::stc_data::WIN_READ_ONLY))
    {
      menuFind->append({{wxID_REPLACE}});
    }
  }
  
  menuFind->append({{wex::ID_TOOL_REPORT_FIND, wex::ellipsed(_("Find &in Files"))}});

  if (!m_app->data().flags().test(wex::stc_data::WIN_READ_ONLY))
  {
    menuFind->append({{wex::ID_TOOL_REPLACE, wex::ellipsed(_("Replace in File&s"))}});
  }

  menuEdit->append({
    {menuFind, !m_app->data().flags().test(wex::stc_data::WIN_READ_ONLY) ?
    _("&Find and Replace"): _("&Find")},
    {}, 
    {wex::ID_EDIT_CONTROL_CHAR, wex::ellipsed(_("&Control Char"), "Ctrl+K"), "", "",
       [=](wxCommandEvent& event) {
       if (get_stc() != nullptr)
       {
         wxPostEvent(get_stc(), event);
       }}},
    {}});
  
  auto* menuMacro = new wex::menu({
    {ID_EDIT_MACRO_START_RECORD, wex::ellipsed(_("Start Record"))},
    {ID_EDIT_MACRO_STOP_RECORD, _("Stop Record")}, {},
    {ID_EDIT_MACRO_PLAYBACK, wex::ellipsed(_("Playback"))}});
  
  if (wex::ex::get_macros().get_filename().file_exists())
  {
    menuMacro->append({
      {}, 
      {ID_EDIT_MACRO, wxGetStockLabel(wxID_EDIT), "", "", [=](wxCommandEvent&) {
         open_file(wex::ex::get_macros().get_filename());}}});
  }
  
  menuEdit->append({{menuMacro, _("&Macro"), std::string(), ID_EDIT_MACRO_MENU}});

  wex::menu* menuDebug = nullptr;

  if (m_app->get_debug())
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

    m_statusbar->show_pane("PaneDBG", true);
  }

  auto* menuOptions = new wex::menu();
  
  if (wex::vcs::size() > 0)
  {
    menuOptions->append({
      {ID_OPTION_VCS, wex::ellipsed(_("Set &VCS")), "", "", [=](wxCommandEvent&) {
         if (wex::vcs().config_dialog() == wxID_OK)
         { 
           wex::vcs vcs;
           vcs.set_entry_from_base(this);
           m_statusbar->show_pane(
             "PaneVCS", 
             vcs.use());
           statustext(vcs.name(), "PaneVCS");
         }}},
      {}});
  }

  menuOptions->append({
#ifndef __WXOSX__
    {wxID_PREFERENCES, wex::ellipsed(_("Set &Editor Options"))},
#else
    {wxID_PREFERENCES},
#endif  
    {}, 
    {ID_OPTION_LIST, wex::ellipsed(_("Set &List Options")), "", "",
        [=](wxCommandEvent& event) {
          wex::listview::config_dialog(wex::window_data().
            title(_("List Options").ToStdString()).
              button(wxOK | wxCANCEL | wxAPPLY).id(ID_OPTION_LIST));}}});

  SetMenuBar(new wex::menubar({
    {new wex::menu({
       {wxID_NEW, 
          wex::ellipsed(wxGetStockLabel(wxID_NEW, wxSTOCK_NOFLAGS), "\tCtrl+N"),
          "", "", [=](wxCommandEvent& event) {
          // In hex mode we cannot edit the file.
          if (wex::config("is_hexmode").get(false)) return;

          static std::string name = event.GetString();
          wxTextEntryDialog dlg(this, _("Input") + ":", _("File Name"), name);
          wxTextValidator validator(wxFILTER_EXCLUDE_CHAR_LIST);
          validator.SetCharExcludes("/\\?%*:|\"<>");
          dlg.SetTextValidator(validator);
          if (dlg.ShowModal() == wxID_CANCEL) return;

          if (name = dlg.GetValue(); !name.empty())
          {
            auto* page = new wex::stc(std::string(),
              wex::stc_data(m_app->data()).window(wex::window_data().
                parent(m_editors)));
            page->get_file().file_new(name);
            // This file does yet exist, so do not give it a bitmap.
            m_editors->add_page(page, name, name, true);
            show_pane("FILES");
          };}},
       {wxID_OPEN}, 
       {}, 
       {ID_RECENT_FILE_MENU, file_history()},
       {wxID_CLOSE, wxGetStockLabel(wxID_CLOSE, wxSTOCK_NOFLAGS) + "\tCtrl+W", 
          "", "", [=](wxCommandEvent& event) {
          auto* editor = get_stc();
          if (editor != nullptr)
          {
            if (!allow_close(m_editors->GetId(), editor)) return;
            m_editors->delete_page(m_editors->key_by_page(editor));
          };},
          [=](wxUpdateUIEvent& event) {
            event.Enable(m_editors->IsShown() && m_editors->GetPageCount() > 0);}},
       {},
       {wxID_SAVE}, 
       {wxID_SAVEAS, "", "", "", nullptr, [=](wxUpdateUIEvent& event) {
          event.Enable(m_editors->IsShown() && m_editors->GetPageCount() > 0);}},
       {wex::ID_ALL_SAVE, _("Save A&ll"), std::string(), wxART_FILE_SAVE}, 
       {},
       {wex::menu_item::PRINT}, 
       {}, 
       {wex::menu_item::EXIT}}), 
       wxGetStockLabel(wxID_FILE)},
    {menuEdit, wxGetStockLabel(wxID_EDIT)},
    {new wex::menu({
       {this}, 
       {},
       {ID_VIEW_FILES, _("&Files"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
          toggle_pane("FILES"); 
          if (!manager().GetPane("FILES").IsShown())
          {
            if (manager().GetPane("PROJECTS").IsShown())
            {
              manager().GetPane("PROJECTS").Maximize();
              manager().Update();
            }
          };},
          [=](wxUpdateUIEvent& event) {
            event.Check(manager().GetPane("FILES").IsShown());}},
       {ID_VIEW_PROJECTS, _("&Projects"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
            toggle_pane("PROJECTS");},
          [=](wxUpdateUIEvent& event) {
            event.Check(
              m_projects != nullptr && 
              manager().GetPane("PROJECTS").IsShown());}},
       {ID_VIEW_DIRCTRL, _("&Explorer"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
            toggle_pane("DIRCTRL");},
          [=](wxUpdateUIEvent& event) {
            event.Check(manager().GetPane("DIRCTRL").IsShown());}},
       {ID_VIEW_HISTORY, _("&History"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
            if (m_history == nullptr)
            {
              add_pane_history();
              manager().Update();
            }
            else
            {
              toggle_pane("HISTORY");
            };},
          [=](wxUpdateUIEvent& event) {
            event.Check(
              m_history != nullptr && 
              manager().GetPane("HISTORY").IsShown());}},
       {ID_VIEW_OUTPUT, _("&Output"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
            toggle_pane("OUTPUT");},
          [=](wxUpdateUIEvent& event) {
            event.Check(manager().GetPane("OUTPUT").IsShown());}}, 
       {},
       {ID_VIEW_ASCII_TABLE, _("&Ascii Table"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
            if (m_ascii_table == nullptr)
            {
              m_ascii_table = new wex::stc();
              manager().AddPane(m_ascii_table, wxAuiPaneInfo()
                .Left()
                .Name("ASCIITABLE")
                .MinSize(500, 150)
                .Caption(_("Ascii Table")));
              manager().Update();
              // Do not show an edge, eol or whitespace for ascii table.
              m_ascii_table->SetEdgeMode(wxSTC_EDGE_NONE);
              m_ascii_table->SetViewEOL(false);
              m_ascii_table->SetViewWhiteSpace(wxSTC_WS_INVISIBLE);
              m_ascii_table->SetTabWidth(5);
              for (int i = 1; i <= 255; i++)
              {
                switch (i)
                {
                  case  9: 
                    m_ascii_table->add_text(wxString::Format("%3d\tTAB", i)); 
                    break;
                  case 10: 
                    m_ascii_table->add_text(wxString::Format("%3d\tLF", i)); 
                    break;
                  case 13: 
                    m_ascii_table->add_text(wxString::Format("%3d\tCR", i)); 
                    break;
                  default:
                    m_ascii_table->add_text(
                      wxString::Format("%3d\t%c", i, (wxUniChar)i));
                }
                m_ascii_table->add_text((i % 5 == 0) ? m_ascii_table->eol(): "\t");
              }
              m_ascii_table->EmptyUndoBuffer();
              m_ascii_table->SetSavePoint();
              m_ascii_table->SetReadOnly(true);
            }
            else
            {
              toggle_pane("ASCIITABLE"); 
            };},
          [=](wxUpdateUIEvent& event) {
            event.Check(
              m_ascii_table != nullptr && 
              manager().GetPane("ASCIITABLE").IsShown());}}}),
     _("&View")},
    {new wex::menu({
       {ID_PROCESS_SELECT, wex::ellipsed(_("&Select")), "", "", 
          [=](wxCommandEvent& event) {
          if (wex::process::config_dialog() == wxID_OK)
          {
            show_pane("PROCESS");
            m_process->execute();
          };}},
       {},
       {wxID_EXECUTE, "", "", "", [=](wxCommandEvent& event) {
          show_pane("PROCESS"); 
          m_process->execute();}},
       {wxID_STOP, "", "", "", [=](wxCommandEvent& event) {
          m_process->stop();
          show_pane("PROCESS", false);}}}),
      _("&Process")},
    {new wex::menu({
       {ID_PROJECT_NEW, wxGetStockLabel(wxID_NEW), std::string(), wxART_NEW, 
          [=](wxCommandEvent& event) {
          const std::string text =
            wxString::Format("%s%d", _("project"), m_project_id++).ToStdString();
          const wex::path fn(
             (!get_project_history().get_history_file().empty() ? 
                 get_project_history().get_history_file().get_path(): 
                 wex::config::dir()),
            text + ".prj");
          wxWindow* page = new wex::report::file(
            fn.string(), 
            wex::window_data().parent(m_projects));
          ((wex::report::file*)page)->file_new(fn.string());
          // This file does yet exist, so do not give it a bitmap.
          m_projects->add_page(page, fn.string(), text, true);
          set_recent_project(fn.string());
          show_pane("PROJECTS");}},
       {ID_PROJECT_OPEN, wxGetStockLabel(wxID_OPEN), std::string(), wxART_FILE_OPEN, 
          [=](wxCommandEvent& event) {
          wxFileDialog dlg(this,
            _("Select Projects"),
             (!get_project_history().get_history_file().empty() ? 
                 get_project_history().get_history_file().get_path(): wex::config::dir()),
            wxEmptyString,
            m_project_wildcard,
            wxFD_OPEN | wxFD_MULTIPLE);
          if (dlg.ShowModal() == wxID_CANCEL) return;
          const std::vector < wex::path > v(
#ifdef __WXOSX__
            {dlg.GetPath().ToStdString()});
#else
            wex::to_vector_path(dlg).get());
#endif
          wex::open_files(this, v, wex::stc_data().flags(
              wex::stc_data::window_t().set(wex::stc_data::WIN_IS_PROJECT)));}},
       {ID_PROJECT_OPENTEXT, _("&Open as Text"), "", "", 
          [=](wxCommandEvent& event) {
          if (auto* project = get_project(); project != nullptr)
          {
            if (wex::file_dialog(project).show_modal_if_changed() != wxID_CANCEL)
            {
              open_file(project->get_filename());
            }
          };}},
       {},
       {ID_RECENT_PROJECT_MENU, get_project_history()},
       {ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE), std::string(), wxART_CLOSE, 
          [=](wxCommandEvent& event) {
          if (auto* project = get_project();
            project != nullptr)
          {
            m_projects->delete_page(project->get_filename().string());
          };}},
       {wex::ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), std::string(), wxART_FILE_SAVE},
       {ID_PROJECT_SAVEAS, wxGetStockLabel(wxID_SAVEAS), std::string(), wxART_FILE_SAVE_AS, 
          [=](wxCommandEvent& event) {
          if (auto* project = get_project();
            project != nullptr && m_projects != nullptr)
          {
            wex::file_dialog dlg(
              project, 
              wex::window_data().
                style(wxFD_SAVE).
                parent(this).
                title(_("Project Save As").ToStdString()),
              m_project_wildcard);
            if (dlg.ShowModal() == wxID_OK)
            {
              project->file_save(dlg.GetPath().ToStdString());
              m_projects->set_page_text(
                m_projects->key_by_page(project),
                project->get_filename().string(),
                project->get_filename().name());
            }
          };}},
       {},
       {ID_SORT_SYNC, _("&Auto Sort"), wex::menu_item::CHECK, 
          [=](wxCommandEvent& event) {
          wex::config("list.SortSync").set(
            !wex::config("list.SortSync").get(true));}}}),
     _("&Project")},
    {menuDebug, _("&Debug")},
    {menuOptions, _("&Options")},
    {new wex::menu({
       {wxID_ABOUT, "", "", "", [=](wxCommandEvent& event) {
          wxAboutDialogInfo info;
          info.SetIcon(GetIcon());
          info.SetVersion(wex::get_version_info().get());
          wxString description(
            _("This program offers a portable text or binary editor\n"
              "with automatic syncing."));
#ifdef __WXMSW__
          description +=
            _(" All its config files are read\n"
              "and saved in the same directory as where the executable is.");
#endif
          info.SetDescription(description);
          info.SetCopyright(wex::get_version_info().copyright());
          info.SetWebSite("http://sourceforge.net/projects/syncped/");
          wxAboutBox(info, this);}}, 
       {wxID_HELP, "", "", "", [=](wxCommandEvent& event) {
          wxLaunchDefaultBrowser(
            "http://antonvw.github.io/syncped/v" + 
            wex::get_version_info().get() + 
            "/syncped.htm");}}}), wxGetStockLabel(wxID_HELP)}}));
}
  
void decorated_frame::add_pane_history()
{
  m_history = new wex::report::listview(
    wex::listview_data().type(wex::listview_data::HISTORY));
        
  manager().AddPane(m_history, wxAuiPaneInfo()
    .Left()
    .MaximizeButton(true)
    .Name("HISTORY")
    .CloseButton(false)
    .MinSize(150, 150)
    .Caption(_("History")));
}

bool decorated_frame::allow_close(wxWindowID id, wxWindow* page)
{
  switch (id)
  {
  case wex::ID_NOTEBOOK_EDITORS:
    if (wex::file_dialog(
      &((wex::stc*)page)->get_file()).show_modal_if_changed() == wxID_CANCEL)
    {
      return false;
    }
  break;
  case wex::ID_NOTEBOOK_PROJECTS:
    if (wex::file_dialog(
       (wex::report::file*)page).show_modal_if_changed() == wxID_CANCEL)
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
    case wex::ID_NOTEBOOK_EDITORS:
      ((wex::stc*)page)->properties_message();
      break;
    case wex::ID_NOTEBOOK_LISTS:
      break;
    case wex::ID_NOTEBOOK_PROJECTS:
      wex::log::status() << ((wex::report::file*)page)->get_filename();
      update_statusbar((wex::report::file*)page);
      break;
    default:
      assert(0);
  }
}
