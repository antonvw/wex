////////////////////////////////////////////////////////////////////////////////
// Name:      support.cpp
// Purpose:   Implementation of DecoratedFrame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/report/listviewfile.h>
#ifndef __WXMSW__
#include "app.xpm"
#endif
#include "support.h"
#include "defs.h"

DecoratedFrame::DecoratedFrame()
  : wxExFrameWithHistory(
      NULL,
      wxID_ANY,
      wxTheApp->GetAppDisplayName(), // title
      25,                            // maxFiles
      4)                             // maxProjects
{
  SetIcon(wxICON(app));

#if wxUSE_STATUSBAR
  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("PaneFileType", 50, _("File type")));
  panes.push_back(wxExStatusBarPane("PaneInfo", 100, _("Lines or items")));

  if (wxExLexers::Get()->GetCount() > 0)
  {
#ifdef __WXMSW__
    const int lexer_size = 60;
#else
    const int lexer_size = 75;
#endif

    panes.push_back(wxExStatusBarPane("PaneLexer", lexer_size, _("Lexer")));
    panes.push_back(wxExStatusBarPane("PaneTheme", lexer_size, _("Theme")));
  }

  if (wxExVCS::GetCount() > 0)
  {
    panes.push_back(wxExStatusBarPane("PaneVCS", 75, _("VCS")));
  }
  
  SetupStatusBar(panes);
  
  if (wxExVCS::GetCount() > 0)
  {
    wxExVCS vcs;
    vcs.GetDir();
    StatusText(vcs.GetName(), "PaneVCS");
  }
#endif

  wxExMenu *menuFile = new wxExMenu();
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENT_FILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_CLOSE);
  menuFile->Append(ID_ALL_STC_CLOSE, _("Close A&ll"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->Append(ID_ALL_STC_SAVE,
    _("Save A&ll"), wxEmptyString, wxART_FILE_SAVE);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu *menuEdit = new wxExMenu();
  menuEdit->Append(wxID_UNDO);
  menuEdit->Append(wxID_REDO);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_CUT);
  menuEdit->Append(wxID_COPY);
  menuEdit->Append(wxID_PASTE);
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_JUMP_TO);
  menuEdit->AppendSeparator();
  wxExMenu* menuFind = new wxExMenu();
  menuFind->Append(wxID_FIND);
  menuFind->Append(wxID_REPLACE);
  menuFind->Append(ID_FIND_IN_FILES, wxExEllipsed(_("Find &In Files")));
  menuFind->Append(ID_REPLACE_IN_FILES, wxExEllipsed(_("Replace In File&s")));
  menuEdit->AppendSubMenu(menuFind, _("&Find And Replace"));
  menuEdit->AppendSeparator();

  wxExMenu* menuMore = new wxExMenu();
  menuMore->Append(ID_EDIT_ADD_HEADER, wxExEllipsed(_("&Add Header")));
  menuMore->Append(ID_EDIT_INSERT_SEQUENCE, wxExEllipsed(_("Insert Sequence")));
  menuMore->AppendSeparator();
  menuMore->Append(
    ID_EDIT_CONTROL_CHAR, wxExEllipsed(_("&Control Char"), "Ctrl+K"));
  
  menuEdit->AppendSubMenu(menuMore, _("More"));
  menuEdit->AppendSeparator();
  
  wxExMenu* menuMacro = new wxExMenu();
  menuMacro->Append(ID_EDIT_MACRO_START_RECORD, _("Start Record"));
  menuMacro->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuMacro->AppendSeparator();
  menuMacro->Append(ID_EDIT_MACRO_PLAYBACK, _("Playback\tCtrl+M"));
  menuEdit->AppendSubMenu(menuMacro, _("&Macro"));

  wxExMenu *menuView = new wxExMenu;
  menuView->AppendBars();
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_FILES, _("&Files"));
  menuView->AppendCheckItem(ID_VIEW_PROJECTS, _("&Projects"));
  menuView->AppendCheckItem(ID_VIEW_DIRCTRL, _("&Explorer"));
  menuView->AppendCheckItem(ID_VIEW_HISTORY, _("&History"));
  menuView->AppendCheckItem(ID_VIEW_OUTPUT, _("&Output"));
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_ASCII_TABLE, _("&Ascii Table"));

  wxMenu *menuProcess = new wxMenu();
  menuProcess->Append(ID_PROCESS_SELECT, wxExEllipsed(_("&Select")));
  menuProcess->AppendSeparator();
  menuProcess->Append(wxID_EXECUTE);
  menuProcess->Append(wxID_STOP);

  wxExMenu *menuProject = new wxExMenu();
  menuProject->Append(
    ID_PROJECT_NEW, wxGetStockLabel(wxID_NEW), wxEmptyString, wxART_NEW);
  menuProject->Append(
    ID_PROJECT_OPEN, wxGetStockLabel(wxID_OPEN), wxEmptyString, wxART_FILE_OPEN);
  UseProjectHistory(ID_RECENT_PROJECT_MENU, menuProject);
  menuProject->Append(ID_PROJECT_OPENTEXT, _("&Open As Text"));
  menuProject->Append(
    ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE), wxEmptyString, wxART_CLOSE);
  menuProject->AppendSeparator();
  menuProject->Append(
    ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), wxEmptyString, wxART_FILE_SAVE);
  menuProject->Append(
    ID_PROJECT_SAVEAS,
    wxGetStockLabel(wxID_SAVEAS), wxEmptyString, wxART_FILE_SAVE_AS);
  menuProject->AppendSeparator();
  menuProject->AppendCheckItem(ID_SORT_SYNC, _("&Auto Sort"));

  wxMenu *menuWindow = new wxMenu();
  menuWindow->Append(ID_SPLIT, _("Split"));

  wxMenu* menuOptions = new wxMenu();
  
  if (wxExVCS::GetCount() > 0)
  {
    menuOptions->Append(ID_OPTION_VCS, wxExEllipsed(_("Set &VCS")));
    menuOptions->AppendSeparator();
  }
  
  if (wxExVCS::GetCount() == 0)
  {
    menuOptions->Append(
      ID_OPTION_COMPARATOR, wxExEllipsed(_("Set &Comparator")));
    menuOptions->AppendSeparator();
  }
  
  menuOptions->Append(ID_OPTION_LIST_FONT, wxExEllipsed(_("Set &List Font")));
  // text also used as caption
  menuOptions->Append(
    ID_OPTION_LIST_READONLY_COLOUR, wxExEllipsed(_("Set List &Read Only Colour")));
  wxMenu *menuListSort = new wxMenu;
  menuListSort->AppendCheckItem(ID_OPTION_LIST_SORT_ASCENDING, _("&Ascending"));
  menuListSort->AppendCheckItem(ID_OPTION_LIST_SORT_DESCENDING, _("&Descending"));
  menuListSort->AppendCheckItem(ID_OPTION_LIST_SORT_TOGGLE, _("&Toggle"));
  menuOptions->AppendSubMenu(menuListSort, _("Set &List Sort Method"));
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_EDITOR, wxExEllipsed(_("Set &Editor Options")));

  wxExMenu *menuHelp = new wxExMenu(); // use wxExMenu for art with HELP
  menuHelp->Append(wxID_ABOUT);
  menuHelp->Append(wxID_HELP);

  wxMenuBar* menubar = new wxMenuBar();
  menubar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menubar->Append(menuEdit, wxGetStockLabel(wxID_EDIT));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuProcess, _("&Process"));
  menubar->Append(menuProject, _("&Project"));
  menubar->Append(menuWindow, _("&Window"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));
  
  SetMenuBar(menubar);
  
  GetToolBar()->AddTool(wxID_HELP);
  GetToolBar()->Realize();
}

bool DecoratedFrame::AllowClose(wxWindowID id, wxWindow* page)
{
  if (id == NOTEBOOK_EDITORS)
  {
    wxExFileDialog dlg(this, &((wxExSTC*)page)->GetFile());
    
    if (dlg.ShowModalIfChanged() != wxID_OK)
    {
      return false;
    }
  }
  else if (id == NOTEBOOK_PROJECTS)
  {
    wxExFileDialog dlg(this, (wxExListViewFile*)page);
    if (dlg.ShowModalIfChanged() != wxID_OK)
    {
      return false;
    }
  }
  
  return wxExFrameWithHistory::AllowClose(id, page);
}

void DecoratedFrame::OnNotebook(wxWindowID id, wxWindow* page)
{
  wxExFrameWithHistory::OnNotebook(id, page);
  
  if (id == NOTEBOOK_EDITORS)
  {
    ((wxExSTC*)page)->PropertiesMessage();
  }
  else if (id == NOTEBOOK_PROJECTS)
  {
#if wxUSE_STATUSBAR
    wxExLogStatus(((wxExListViewFile*)page)->GetFileName());
    UpdateStatusBar((wxExListViewFile*)page);
#endif
  }
  else if (id == NOTEBOOK_LISTS)
  {
    // Do nothing special.
  }
  else
  {
    wxFAIL;
  }
}
