////////////////////////////////////////////////////////////////////////////////
// Name:      support.cpp
// Purpose:   Implementation of decorated_frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/debug.h>
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/menu.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/report/listviewfile.h>
#ifndef __WXMSW__
#include "app.xpm"
#endif
#include "support.h"
#include "app.h"
#include "defs.h"

decorated_frame::decorated_frame(app* app)
  : wex::history_frame(
      25,  // maxFiles
      4,   // maxProjects
      wex::window_data().Name("mainFrame").Style(wxDEFAULT_FRAME_STYLE))
  , m_App(app)
{
  SetIcon(wxICON(app));
  
  wex::process::PrepareOutput(this);

  const bool vi_mode = wxConfigBase::Get()->ReadBool(_("vi mode"), false);
  
#ifdef __WXMSW__
  const int lexer_size = 60;
#else
  const int lexer_size = 75;
#endif
  SetupStatusBar({
    {"PaneFileType", 50, _("File type").ToStdString()},
    {"PaneInfo", 100, _("Lines or items").ToStdString()},
    {"PaneLexer", lexer_size, _("Lexer").ToStdString()},
    {"PaneTheme", lexer_size, _("Theme").ToStdString()},
    {"PaneVCS", 150},
    {"PaneMacro", 75},
    {"PaneMode", 100}});
  
  if (wex::vcs vcs; vcs.Use() && wex::vcs::GetCount() > 0)
  {
    const auto b(vcs.SetEntryFromBase() ? vcs.GetBranch(): std::string());
    StatusText(!b.empty() ? b: vcs.GetName(), "PaneVCS");
  }
  else
  {
    m_StatusBar->ShowField("PaneVCS", false);
  }
  
  if (wex::lexers::Get()->GetLexers().empty())
  {
    m_StatusBar->ShowField("PaneLexer", false);
    m_StatusBar->ShowField("PaneTheme", false);
  }
  
  m_StatusBar->ShowField("PaneMacro", vi_mode);
  m_StatusBar->ShowField("PaneMode", false);

  auto *menuFile = new wex::menu();
  menuFile->Append(wxID_NEW,
    wex::ellipsed(wxGetStockLabel(wxID_NEW, wxSTOCK_NOFLAGS), "\tCtrl+N"));
  menuFile->Append(wxID_OPEN);
  GetFileHistory().UseMenu(ID_RECENT_FILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_CLOSE,
    wxGetStockLabel(wxID_CLOSE, wxSTOCK_NOFLAGS) + "\tCtrl+W");
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->Append(wex::ID_ALL_SAVE,
    _("Save A&ll"), std::string(), wxART_FILE_SAVE);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  auto *menuEdit = new wex::menu();
  menuEdit->Append(wxID_UNDO);
  menuEdit->Append(wxID_REDO);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_CUT);
  menuEdit->Append(wxID_COPY);
  menuEdit->Append(wxID_PASTE);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_JUMP_TO);
  menuEdit->Append(wxID_CLEAR);
  menuEdit->Append(wxID_SELECTALL);
  menuEdit->AppendSeparator();
  auto* menuFind = new wex::menu();
  
  if (vi_mode)
  {
    // No accelerators for vi mode, Ctrl F is page down.
    menuFind->Append(wxID_FIND, wxGetStockLabel(wxID_FIND, wxSTOCK_NOFLAGS));

    if (!(m_App->GetData().Flags() & wex::STC_WIN_READ_ONLY))
    {
      menuFind->Append(wxID_REPLACE, wxGetStockLabel(wxID_REPLACE, wxSTOCK_NOFLAGS));
    }
  }
  else
  {
    menuFind->Append(wxID_FIND);

    if (!(m_App->GetData().Flags() & wex::STC_WIN_READ_ONLY))
    {
      menuFind->Append(wxID_REPLACE);
    }
  }
  
  menuFind->Append(wex::ID_TOOL_REPORT_FIND, wex::ellipsed(_("Find &in Files")));

  if (!(m_App->GetData().Flags() & wex::STC_WIN_READ_ONLY))
  {
    menuFind->Append(wex::ID_TOOL_REPLACE, wex::ellipsed(_("Replace in File&s")));
  }

  menuEdit->AppendSubMenu(menuFind, !(m_App->GetData().Flags() & wex::STC_WIN_READ_ONLY) ?
    _("&Find and Replace"): _("&Find"));
  menuEdit->AppendSeparator();
  menuEdit->Append(
    wex::ID_EDIT_CONTROL_CHAR, wex::ellipsed(_("&Control Char"), "Ctrl+K"));
  menuEdit->AppendSeparator();
  
  auto* menuMacro = new wex::menu();
  menuMacro->Append(ID_EDIT_MACRO_START_RECORD, wex::ellipsed(_("Start Record")));
  menuMacro->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuMacro->AppendSeparator();
  menuMacro->Append(ID_EDIT_MACRO_PLAYBACK, wex::ellipsed(_("Playback"), "Ctrl+M"));
  
  if (wex::vi_macros::GetFileName().FileExists())
  {
    menuMacro->AppendSeparator();
    menuMacro->Append(ID_EDIT_MACRO, wxGetStockLabel(wxID_EDIT));
  }
  
  menuEdit->AppendSubMenu(menuMacro, _("&Macro"), std::string(), ID_EDIT_MACRO_MENU);

  auto *menuView = new wex::menu;
  AppendPanes(menuView);
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_FILES, _("&Files"));
  menuView->AppendCheckItem(ID_VIEW_PROJECTS, _("&Projects"));
  menuView->AppendCheckItem(ID_VIEW_DIRCTRL, _("&Explorer"));
  menuView->AppendCheckItem(ID_VIEW_HISTORY, _("&History"));
  menuView->AppendCheckItem(ID_VIEW_OUTPUT, _("&Output"));
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_ASCII_TABLE, _("&Ascii Table"));

  auto *menuProcess = new wex::menu();
  menuProcess->Append(ID_PROCESS_SELECT, wex::ellipsed(_("&Select")));
  menuProcess->AppendSeparator();
  menuProcess->Append(wxID_EXECUTE);
  menuProcess->Append(wxID_STOP);

  auto *menuProject = new wex::menu();
  menuProject->Append(
    ID_PROJECT_NEW, wxGetStockLabel(wxID_NEW), std::string(), wxART_NEW);
  menuProject->Append(
    ID_PROJECT_OPEN, wxGetStockLabel(wxID_OPEN), std::string(), wxART_FILE_OPEN);
  GetProjectHistory().UseMenu(ID_RECENT_PROJECT_MENU, menuProject);
  menuProject->Append(ID_PROJECT_OPENTEXT, _("&Open as Text"));
  menuProject->AppendSeparator();
  menuProject->Append(
    ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE), std::string(), wxART_CLOSE);
  menuProject->AppendSeparator();
  menuProject->Append(
    wex::ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), std::string(), wxART_FILE_SAVE);
  menuProject->Append(
    ID_PROJECT_SAVEAS,
    wxGetStockLabel(wxID_SAVEAS), std::string(), wxART_FILE_SAVE_AS);
  menuProject->AppendSeparator();
  menuProject->AppendCheckItem(ID_SORT_SYNC, _("&Auto Sort"));
  
  wex::menu* menuDebug = nullptr;

  if (m_App->GetDebug())
  {
    menuDebug = new wex::menu();

    if (GetDebug()->AddMenu(menuDebug) == 0)
    {
      delete menuDebug;
      menuDebug = nullptr;
      wex::log() << "no debug menu present";
    }
  }

  wxMenu* menuOptions = new wxMenu();
  
  if (wex::vcs::GetCount() > 0)
  {
    menuOptions->Append(ID_OPTION_VCS, wex::ellipsed(_("Set &VCS")));
    menuOptions->AppendSeparator();
  }

#ifndef __WXOSX__
  menuOptions->Append(wxID_PREFERENCES, wex::ellipsed(_("Set &Editor Options")));
#else
  menuOptions->Append(wxID_PREFERENCES);
#endif  
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_LIST, wex::ellipsed(_("Set &List Options")));

  auto *menuHelp = new wex::menu();
  menuHelp->Append(wxID_ABOUT);
  menuHelp->Append(wxID_HELP);

  wxMenuBar* menubar = new wxMenuBar();
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

bool decorated_frame::AllowClose(wxWindowID id, wxWindow* page)
{
  switch (id)
  {
  case wex::ID_NOTEBOOK_EDITORS:
    if (wex::file_dialog(
      &((wex::stc*)page)->GetFile()).ShowModalIfChanged() == wxID_CANCEL)
    {
      return false;
    }
  break;
  case wex::ID_NOTEBOOK_PROJECTS:
    if (wex::file_dialog(
       (wex::listview_file*)page).ShowModalIfChanged() == wxID_CANCEL)
    {
      return false;
    }
  break;
  }
  
  return wex::history_frame::AllowClose(id, page);
}

void decorated_frame::OnNotebook(wxWindowID id, wxWindow* page)
{
  wex::history_frame::OnNotebook(id, page);
  
  switch (id)
  {
    case wex::ID_NOTEBOOK_EDITORS:
      ((wex::stc*)page)->PropertiesMessage();
    break;
    case wex::ID_NOTEBOOK_LISTS:
    break;
    case wex::ID_NOTEBOOK_PROJECTS:
      wex::log_status(((wex::listview_file*)page)->GetFileName());
      UpdateStatusBar((wex::listview_file*)page);
    break;
    default:
      wxFAIL;
  }
}
