/******************************************************************************\
* File:          support.cpp
* Purpose:       Implementation of support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/vcs.h>
#include <wx/extension/util.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/process.h>
#include <wx/extension/report/stc.h>
#ifndef __WXMSW__
#include "app.xpm"
#endif
#include "support.h"
#include "defs.h"

Frame::Frame()
  : wxExFrameWithHistory(
      NULL,
      wxID_ANY,
      wxTheApp->GetAppName(), // title
      25,                     // maxFiles
      4)                      // maxProjects
{
  SetIcon(wxICON(app));

#if wxUSE_STATUSBAR
  std::vector<wxExPane> panes;
  panes.push_back(wxExPane("PaneText", -3));
  panes.push_back(wxExPane("PaneFileType", 50, _("File Type")));
  panes.push_back(wxExPane("PaneLines", 100, _("Lines")));

  // Add the lexer pane only if we have lexers.
  wxExLexers* lexers = wxExLexers::Get();

  if (lexers->Count() > 0)
  {
#ifdef __WXMSW__
    const int lexer_size = 60;
#else
    const int lexer_size = 75;
#endif
    panes.push_back(wxExPane("PaneLexer", lexer_size, _("Lexer")));
  }

  panes.push_back(wxExPane("PaneItems", 65, _("Items")));
  SetupStatusBar(panes);
#endif

  wxMenuBar* menubar = new wxMenuBar(wxMB_DOCKABLE); // wxMB_DOCKABLE only used for GTK
  SetMenuBar(menubar);

  wxExMenu *menuFile = new wxExMenu();
  menuFile->Append(wxID_NEW);
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENT_FILE_MENU, menuFile);
  menuFile->Append(ID_OPEN_LEXERS, _("Open &Lexers"));
  menuFile->Append(ID_OPEN_LOGFILE, _("Open &Logfile"));
  menuFile->Append(wxID_CLOSE);
  menuFile->Append(ID_ALL_STC_CLOSE, _("Close A&ll"));
  menuFile->AppendSeparator();
  menuFile->Append(wxID_SAVE);
  menuFile->Append(wxID_SAVEAS);
  menuFile->Append(ID_ALL_STC_SAVE, _("Save A&ll"), wxEmptyString, wxART_FILE_SAVE);
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
  menuEdit->Append(wxID_FIND);
  menuEdit->Append(ID_EDIT_FIND_NEXT, _("Find &Next\tF3"));
  menuEdit->Append(wxID_REPLACE);
  menuEdit->Append(ID_FIND_IN_FILES, wxExEllipsed(_("Find &In Files")));
  menuEdit->Append(ID_REPLACE_IN_FILES, wxExEllipsed(_("Replace In File&s")));
  menuEdit->AppendSeparator();
  menuEdit->AppendTools();
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_JUMP_TO);
  menuEdit->AppendSeparator();
  menuEdit->Append(ID_EDIT_CONTROL_CHAR, wxExEllipsed(_("&Control Char"), "Ctrl-H"));
  menuEdit->AppendSeparator();

  if (wxExVCS::Get()->Use())
  {
    wxExMenu* menuVCS = new wxExMenu;
    menuVCS->AppendVCS(ID_VCS_STAT);
    menuVCS->AppendVCS(ID_VCS_INFO);
    menuVCS->AppendVCS(ID_VCS_LOG);
    menuVCS->AppendVCS(ID_VCS_LS);
    menuVCS->AppendVCS(ID_VCS_DIFF);
    menuVCS->AppendVCS(ID_VCS_HELP);
    menuVCS->AppendSeparator();
    menuVCS->AppendVCS(ID_VCS_UPDATE);
    menuVCS->AppendVCS(ID_VCS_COMMIT);
    menuVCS->AppendSeparator();
    menuVCS->AppendVCS(ID_VCS_ADD);
    menuEdit->AppendSubMenu(menuVCS, "&VCS");
    menuEdit->AppendSeparator();
  }

  menuEdit->Append(ID_EDIT_MACRO_START_RECORD, _("Start Record"));
  menuEdit->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuEdit->Append(ID_EDIT_MACRO_PLAYBACK, _("Playback\tCtrl-M"));

  wxMenu *menuView = new wxMenu;
  menuView->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));
  menuView->AppendCheckItem(ID_VIEW_TOOLBAR, _("&Toolbar"));
  menuView->AppendCheckItem(ID_VIEW_MENUBAR, _("&Menubar"));
  menuView->AppendCheckItem(ID_VIEW_FINDBAR, _("&Findbar"));
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
  menuProject->Append(ID_PROJECT_NEW, wxGetStockLabel(wxID_NEW), wxEmptyString, wxART_NEW);
  menuProject->Append(ID_PROJECT_OPEN, wxGetStockLabel(wxID_OPEN), wxEmptyString, wxART_FILE_OPEN);
  UseProjectHistory(ID_RECENT_PROJECT_MENU, menuProject);
  menuProject->Append(ID_PROJECT_OPENTEXT, _("&Open As Text"));
  menuProject->Append(ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE), wxEmptyString, wxART_CLOSE);
  menuProject->AppendSeparator();
  menuProject->Append(ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), wxEmptyString, wxART_FILE_SAVE);
  menuProject->Append(ID_PROJECT_SAVEAS, wxGetStockLabel(wxID_SAVEAS), wxEmptyString, wxART_FILE_SAVE_AS);
  menuProject->AppendSeparator();
  menuProject->AppendCheckItem(ID_SORT_SYNC, _("&Auto Sort"));

  wxMenu *menuWindow = new wxMenu();
  menuWindow->Append(ID_SPLIT, _("Split"));

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(ID_OPTION_VCS_AND_COMPARATOR, wxExEllipsed(_("Set VCS And &Comparator")));
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_LIST_FONT, wxExEllipsed(_("Set &List Font")));
  // text also used as caption
  menuOptions->Append(ID_OPTION_LIST_READONLY_COLOUR, wxExEllipsed(_("Set List Read Only Colour")));
  wxMenu *menuListSort = new wxMenu;
  menuListSort->AppendCheckItem(ID_OPTION_LIST_SORT_ASCENDING, _("&Ascending"));
  menuListSort->AppendCheckItem(ID_OPTION_LIST_SORT_DESCENDING, _("&Descending"));
  menuListSort->AppendCheckItem(ID_OPTION_LIST_SORT_TOGGLE, _("&Toggle"));
  menuOptions->AppendSubMenu(menuListSort, _("Set &List Sort Method"));
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_EDITOR, wxExEllipsed(_("Set &Editor Options")));

  wxMenu *menuHelp = new wxMenu();
  menuHelp->Append(wxID_ABOUT);

  menubar->Append(menuFile, wxGetStockLabel(wxID_FILE));
  menubar->Append(menuEdit, wxGetStockLabel(wxID_EDIT));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuProcess, _("&Process"));
  menubar->Append(menuProject, _("&Project"));
  menubar->Append(menuWindow, _("&Window"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));

  CreateToolBar();

  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->AddTool(wxID_SAVE);
  m_ToolBar->AddTool(wxID_PRINT);
  m_ToolBar->AddSeparator();
  m_ToolBar->AddTool(wxID_FIND);
  
#ifdef __WXGTK__
  // wxID_EXECUTE is not part of art provider, but GTK directly,
  // so the following does not present a bitmap.
  //m_ToolBar->AddSeparator();
  //m_ToolBar->AddTool(wxID_EXECUTE);
#endif

  m_ToolBar->AddSeparator();
  ((wxToolBar*)m_ToolBar)->AddTool(
    ID_PROJECT_OPEN,
    wxEmptyString,
    wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, m_ToolBar->GetToolBitmapSize()),
    wxExEllipsed(_("Open project")));
  ((wxToolBar*)m_ToolBar)->AddTool(
    ID_PROJECT_SAVE,
    wxEmptyString,
    wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, m_ToolBar->GetToolBitmapSize()),
    _("Save project"));

#if wxUSE_CHECKBOX
  m_ToolBar->AddSeparator();

  m_ToolBar->AddControl(
    m_HexModeCheckBox = new wxCheckBox(
      m_ToolBar,
      ID_EDIT_HEX_MODE,
      "Hex",
      wxDefaultPosition,
      wxSize(-1, m_ToolBar->GetToolSize().GetHeight())));

  m_ToolBar->AddControl(
    m_SyncCheckBox = new wxCheckBox(
      m_ToolBar,
      ID_SYNC_MODE,
      "Sync",
      wxDefaultPosition,
      wxSize(-1, m_ToolBar->GetToolSize().GetHeight())));

#if wxUSE_TOOLTIPS
  m_HexModeCheckBox->SetToolTip(_("View in hex mode"));
#endif
  m_HexModeCheckBox->SetValue(wxConfigBase::Get()->ReadBool("HexMode", false)); // default no hex
#if wxUSE_TOOLTIPS
  m_SyncCheckBox->SetToolTip(_("Synchronize modified files"));
#endif
  m_SyncCheckBox->SetValue(wxConfigBase::Get()->ReadBool("AllowSync", true));
#endif // wxUSE_CHECKBOX

  m_ToolBar->Realize();
}

bool Frame::AllowClose(wxWindowID id, wxWindow* page)
{
  if (wxExProcess::Get()->IsRunning())
  {
    return false;
  }
  else if (id == NOTEBOOK_EDITORS)
  {
    wxExFileDialog dlg(this, (wxExSTCWithFrame*)page);
    return dlg.ShowModalIfChanged() == wxID_OK;
  }
  else if (id == NOTEBOOK_PROJECTS)
  {
    wxExFileDialog dlg(this, (wxExListViewFile*)page);
    return dlg.ShowModalIfChanged() == wxID_OK;
  }
  else
  {
    return wxExFrameWithHistory::AllowClose(id, page);
  }
}

void Frame::OnNotebook(wxWindowID id, wxWindow* page)
{
  if (id == NOTEBOOK_EDITORS)
  {
    ((wxExSTCWithFrame*)page)->PropertiesMessage();
  }
  else if (id == NOTEBOOK_PROJECTS)
  {
    SetTitle(wxEmptyString, ((wxExListViewFile*)page)->GetFileName().GetName());
#if wxUSE_STATUSBAR
    StatusText(((wxExListViewFile*)page)->GetFileName());
    ((wxExListViewFile*)page)->UpdateStatusBar();
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
