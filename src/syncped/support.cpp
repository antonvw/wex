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
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wex/config.h>
#include <wex/debug.h>
#include <wex/filedlg.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/menu.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/vi-macros.h>
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
  , m_App(app)
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
    {"PaneVCS", 150},
    {"PaneDBG", 50, "Debugger"},
    {"PaneMacro", 75},
    {"PaneMode", 100}});
  
  if (wex::vcs vcs; vcs.use() && wex::vcs::size() > 0)
  {
    const auto b(vcs.set_entry_from_base() ? vcs.get_branch(): std::string());
    statustext(!b.empty() ? b: vcs.name(), "PaneVCS");
  }
  else
  {
    m_StatusBar->show_field("PaneVCS", false);
  }
  
  if (wex::lexers::get()->get_lexers().empty())
  {
    m_StatusBar->show_field("PaneLexer", false);
    m_StatusBar->show_field("PaneTheme", false);
  }
  
  const bool vi_mode = wex::config(_("vi mode")).get(false);
  
  m_StatusBar->show_field("PaneMacro", vi_mode);
  m_StatusBar->show_field("PaneMode", false);

  auto *menuFile = new wex::menu();
  menuFile->append(wxID_NEW,
    wex::ellipsed(wxGetStockLabel(wxID_NEW, wxSTOCK_NOFLAGS), "\tCtrl+N"));
  menuFile->append(wxID_OPEN);
  file_history().use_menu(ID_RECENT_FILE_MENU, menuFile);
  menuFile->append_separator();
  menuFile->append(wxID_CLOSE,
    wxGetStockLabel(wxID_CLOSE, wxSTOCK_NOFLAGS) + "\tCtrl+W");
  menuFile->append_separator();
  menuFile->append(wxID_SAVE);
  menuFile->append(wxID_SAVEAS);
  menuFile->append(wex::ID_ALL_SAVE,
    _("Save A&ll"), std::string(), wxART_FILE_SAVE);
  menuFile->append_separator();
  menuFile->append_print();
  menuFile->append_separator();
  menuFile->append(wxID_EXIT);

  auto *menuEdit = new wex::menu();
  menuEdit->append(wxID_UNDO);
  menuEdit->append(wxID_REDO);
  menuEdit->append_separator();
  menuEdit->append(wxID_CUT);
  menuEdit->append(wxID_COPY);
  menuEdit->append(wxID_PASTE);
  menuEdit->append_separator();
  menuEdit->append(wxID_JUMP_TO);
  menuEdit->append(wxID_CLEAR);
  menuEdit->append(wxID_SELECTALL);
  menuEdit->append_separator();
  auto* menuFind = new wex::menu();
  
  if (vi_mode)
  {
    // No accelerators for vi mode, Ctrl F is page down.
    menuFind->append(wxID_FIND, wxGetStockLabel(wxID_FIND, wxSTOCK_NOFLAGS));

    if (m_App->data().flags().test(wex::stc_data::WIN_READ_ONLY))
    {
      menuFind->append(wxID_REPLACE, wxGetStockLabel(wxID_REPLACE, wxSTOCK_NOFLAGS));
    }
  }
  else
  {
    menuFind->append(wxID_FIND);

    if (!m_App->data().flags().test(wex::stc_data::WIN_READ_ONLY))
    {
      menuFind->append(wxID_REPLACE);
    }
  }
  
  menuFind->append(wex::ID_TOOL_REPORT_FIND, wex::ellipsed(_("Find &in Files")));

  if (!m_App->data().flags().test(wex::stc_data::WIN_READ_ONLY))
  {
    menuFind->append(wex::ID_TOOL_REPLACE, wex::ellipsed(_("Replace in File&s")));
  }

  menuEdit->append_submenu(menuFind, !m_App->data().flags().test(wex::stc_data::WIN_READ_ONLY) ?
    _("&Find and Replace"): _("&Find"));
  menuEdit->append_separator();
  menuEdit->append(
    wex::ID_EDIT_CONTROL_CHAR, wex::ellipsed(_("&Control Char"), "Ctrl+K"));
  menuEdit->append_separator();
  
  auto* menuMacro = new wex::menu();
  menuMacro->append(ID_EDIT_MACRO_START_RECORD, wex::ellipsed(_("Start Record")));
  menuMacro->append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuMacro->append_separator();
  menuMacro->append(ID_EDIT_MACRO_PLAYBACK, wex::ellipsed(_("Playback")));
  
  if (wex::vi_macros::get_filename().file_exists())
  {
    menuMacro->append_separator();
    menuMacro->append(ID_EDIT_MACRO, wxGetStockLabel(wxID_EDIT));
  }
  
  menuEdit->append_submenu(menuMacro, _("&Macro"), std::string(), ID_EDIT_MACRO_MENU);

  auto *menuView = new wex::menu;
  append_panes(menuView);
  menuView->append_separator();
  menuView->AppendCheckItem(ID_VIEW_FILES, _("&Files"));
  menuView->AppendCheckItem(ID_VIEW_PROJECTS, _("&Projects"));
  menuView->AppendCheckItem(ID_VIEW_DIRCTRL, _("&Explorer"));
  menuView->AppendCheckItem(ID_VIEW_HISTORY, _("&History"));
  menuView->AppendCheckItem(ID_VIEW_OUTPUT, _("&Output"));
  menuView->append_separator();
  menuView->AppendCheckItem(ID_VIEW_ASCII_TABLE, _("&Ascii Table"));

  auto *menuProcess = new wex::menu();
  menuProcess->append(ID_PROCESS_SELECT, wex::ellipsed(_("&Select")));
  menuProcess->append_separator();
  menuProcess->append(wxID_EXECUTE);
  menuProcess->append(wxID_STOP);

  auto *menuProject = new wex::menu();
  menuProject->append(
    ID_PROJECT_NEW, wxGetStockLabel(wxID_NEW), std::string(), wxART_NEW);
  menuProject->append(
    ID_PROJECT_OPEN, wxGetStockLabel(wxID_OPEN), std::string(), wxART_FILE_OPEN);
  get_project_history().use_menu(ID_RECENT_PROJECT_MENU, menuProject);
  menuProject->append(ID_PROJECT_OPENTEXT, _("&Open as Text"));
  menuProject->append_separator();
  menuProject->append(
    ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE), std::string(), wxART_CLOSE);
  menuProject->append_separator();
  menuProject->append(
    wex::ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), std::string(), wxART_FILE_SAVE);
  menuProject->append(
    ID_PROJECT_SAVEAS,
    wxGetStockLabel(wxID_SAVEAS), std::string(), wxART_FILE_SAVE_AS);
  menuProject->append_separator();
  menuProject->AppendCheckItem(ID_SORT_SYNC, _("&Auto Sort"));
  
  wex::menu* menuDebug = nullptr;

  if (m_App->get_debug())
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
  }
  else
  {
    m_StatusBar->show_field("PaneDBG", false);
  }

  auto* menuOptions = new wex::menu();
  
  if (wex::vcs::size() > 0)
  {
    menuOptions->append(ID_OPTION_VCS, wex::ellipsed(_("Set &VCS")));
    menuOptions->append_separator();
  }

#ifndef __WXOSX__
  menuOptions->append(wxID_PREFERENCES, wex::ellipsed(_("Set &Editor Options")));
#else
  menuOptions->append(wxID_PREFERENCES);
#endif  
  menuOptions->append_separator();
  menuOptions->append(ID_OPTION_LIST, wex::ellipsed(_("Set &List Options")));

  auto *menuHelp = new wex::menu();
  menuHelp->append(wxID_ABOUT);
  menuHelp->append(wxID_HELP);

  auto* menubar = new wxMenuBar();
  menubar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menubar->Append(menuEdit, wxGetStockLabel(wxID_EDIT));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuProcess, _("&Process"));
  menubar->Append(menuProject, _("&Project"));
  if (menuDebug != nullptr)
    menubar->Append(menuDebug, _("&Debug"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  
  SetMenuBar(menubar);
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
