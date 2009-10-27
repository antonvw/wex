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

#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/extension/filedlg.h>
#include <wx/extension/lexers.h>
#ifndef __WXMSW__
#include "appl.xpm"
#endif
#include "support.h"
#include "defs.h"

Frame::Frame()
  : wxExFrameWithHistory(
      NULL,
      wxID_ANY,
      wxTheApp->GetAppName(), //title
      NUMBER_RECENT_FILES,    //maxFiles
      4)                      // maxProjects
{
  SetIcon(wxICON(appl));

  std::vector<wxExPane> panes;
  panes.push_back(wxExPane("PaneText", -3));
  panes.push_back(wxExPane("PaneFileType", 50, _("File Type")));
  panes.push_back(wxExPane("PaneLines", 100, _("Lines")));

  // Add the lexer pane only if we have lexers.
  if (wxExLexers::Get()->Count() > 0)
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
  menuEdit->Append(ID_SPECIAL_FIND_IN_FILES, wxExEllipsed(_("Find &In Files")));
  menuEdit->Append(ID_SPECIAL_REPLACE_IN_FILES, wxExEllipsed(_("Replace In File&s")));
  menuEdit->AppendSeparator();
  menuEdit->AppendTools();
  menuEdit->AppendSeparator();
  menuEdit->Append(wxID_JUMP_TO);
  menuEdit->AppendSeparator();
  menuEdit->Append(ID_EDIT_CONTROL_CHAR, wxExEllipsed(_("&Control Char"), "Ctrl+H"));
  menuEdit->AppendSeparator();

  if (wxConfigBase::Get()->ReadBool("SVN", true))
  {
    wxMenu* menuSVN = new wxMenu;
    menuSVN->Append(ID_SVN_STAT, wxExEllipsed("&Stat"));
    menuSVN->Append(ID_SVN_INFO, wxExEllipsed("&Info"));
    menuSVN->Append(ID_SVN_LOG, wxExEllipsed("&Log"));
    menuSVN->Append(ID_SVN_DIFF, wxExEllipsed("&Diff"));
    menuSVN->Append(ID_SVN_HELP, wxExEllipsed("&Help"));
    menuSVN->AppendSeparator();
    menuSVN->Append(ID_SVN_UPDATE, wxExEllipsed("&Update"));
    menuSVN->Append(ID_SVN_COMMIT, wxExEllipsed("C&ommit"));
    menuEdit->AppendSubMenu(menuSVN, "&SVN");
    menuEdit->AppendSeparator();
  }

  menuEdit->Append(ID_EDIT_MACRO_START_RECORD, _("Start Record"));
  menuEdit->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuEdit->Append(ID_EDIT_MACRO_PLAYBACK, _("Playback\tCtrl+M"));

  wxMenu *menuView = new wxMenu;
  menuView->AppendCheckItem(ID_VIEW_STATUSBAR, _("&Statusbar"));
  menuView->AppendCheckItem(ID_VIEW_TOOLBAR, _("&Toolbar"));
  menuView->AppendCheckItem(ID_VIEW_FINDBAR, _("&Findbar"));
  menuView->AppendSeparator();
  menuView->AppendCheckItem(ID_VIEW_FILES, _("&Files"));
  menuView->AppendCheckItem(ID_VIEW_PROJECTS, _("&Projects"));
  menuView->AppendCheckItem(ID_VIEW_DIRCTRL, _("&Explorer"));
  menuView->AppendCheckItem(ID_VIEW_OUTPUT, _("&Output"));
  menuView->AppendCheckItem(ID_VIEW_ASCII_TABLE, _("&Ascii Table"));
  menuView->AppendCheckItem(ID_VIEW_HISTORY, _("&History"));
#ifdef __WXMSW__
  wxMenu *menuListView = new wxMenu;
  menuListView->AppendCheckItem(wxID_VIEW_LIST, _("&List"));
  menuListView->AppendCheckItem(wxID_VIEW_DETAILS, _("&Detail"));
  menuListView->AppendCheckItem(wxID_VIEW_SMALLICONS, _("&Small Icon"));
  menuView->AppendSeparator();
  menuView->AppendSubMenu(menuListView, _("&Lists"));
#endif

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
  menuProject->Append(ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE));
  menuProject->AppendSeparator();
  menuProject->Append(ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), wxEmptyString, wxART_FILE_SAVE);
  menuProject->Append(ID_PROJECT_SAVEAS, wxGetStockLabel(wxID_SAVEAS), wxEmptyString, wxART_FILE_SAVE_AS);
  menuProject->AppendSeparator();
  menuProject->AppendCheckItem(ID_SORT_SYNC, _("&Auto Sort"));

  wxMenu *menuWindow = new wxMenu();
  menuWindow->Append(ID_SPLIT, _("Split"));

  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(ID_OPTION_SVN_AND_COMPARATOR, wxExEllipsed(_("Set SVN And &Comparator")));
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_LIST_FONT, wxExEllipsed(_("Set &List Font")));
  menuOptions->Append(ID_OPTION_LIST_READONLY_COLOUR, wxExEllipsed(_("Set &List Read Only Colour")));
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

#ifdef __WXMSW__
  m_ToolBar->AddSeparator();
  m_ToolBar->AddCheckTool(
    wxID_VIEW_LIST,
    wxEmptyString,
    wxArtProvider::GetBitmap(wxART_LIST_VIEW, wxART_TOOLBAR, m_ToolBar->GetToolBitmapSize()),
    wxNullBitmap,
    _("View in list mode"));
  m_ToolBar->AddCheckTool(
    wxID_VIEW_DETAILS, wxEmptyString,
    wxArtProvider::GetBitmap(wxART_REPORT_VIEW, wxART_TOOLBAR, m_ToolBar->GetToolBitmapSize()),
    wxNullBitmap,
    _("View in detail mode"));
#endif //__WXMSW__

#if wxUSE_CHECKBOX
  m_ToolBar->AddSeparator();
#ifndef __WXMSW__
  wxSize size(55, 25);
#else
  wxSize size = wxDefaultSize;
#endif // __WXMSW__

  m_ToolBar->AddControl(
    m_HexModeCheckBox = new wxCheckBox(
      m_ToolBar,
      ID_EDIT_HEX_MODE,
      "Hex",
      wxDefaultPosition,
      size,
      wxNO_BORDER));

  m_ToolBar->AddControl(
    m_SyncCheckBox = new wxCheckBox(
      m_ToolBar,
      ID_SYNC_MODE,
      "Sync",
      wxDefaultPosition,
      size,
      wxNO_BORDER));

  m_HexModeCheckBox->SetToolTip(_("View in hex mode"));
  m_HexModeCheckBox->SetValue(wxConfigBase::Get()->ReadBool("HexMode", false)); // default no hex
  m_SyncCheckBox->SetToolTip(_("Synchronize modified files"));
  m_SyncCheckBox->SetValue(wxConfigBase::Get()->ReadBool("AllowSync", true));
#endif // wxUSE_CHECKBOX

  m_ToolBar->Realize();
}

bool Frame::AllowClose(wxWindowID id, wxWindow* page)
{
  if (ProcessIsRunning())
  {
    return false;
  }
  else if (id == NOTEBOOK_EDITORS)
  {
    wxExFileDialog dlg(this, (wxExSTCWithFrame*)page);
    return dlg.Continue();
  }
  else if (id == NOTEBOOK_PROJECTS)
  {
    wxExFileDialog dlg(this, (wxExListViewWithFrame*)page);
    return dlg.Continue();
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
    SetTitle(wxEmptyString, ((wxExListViewWithFrame*)page)->GetFileName().GetName());
#if wxUSE_STATUSBAR
    StatusText(((wxExListViewWithFrame*)page)->GetFileName());
    ((wxExListViewWithFrame*)page)->UpdateStatusBar();
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
