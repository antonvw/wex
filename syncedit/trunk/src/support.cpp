/******************************************************************************\
* File:          support.cpp
* Purpose:       Implementation of support classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stockitem.h> // for wxGetStockLabel
#ifndef __WXMSW__
#include "appl.xpm"
#endif
#include "support.h"
#include "defs.h"

Frame::Frame(const wxString& project_wildcard)
  : ftFrame(
      NULL, 
      wxID_ANY, 
      wxTheApp->GetAppName(), //title
      NUMBER_RECENT_FILES, //maxFiles
      4,    // maxProjects
      project_wildcard)
{
  SetIcon(wxICON(appl));

  std::vector<exPane> panes;
  panes.push_back(exPane("PaneText", -3));
  panes.push_back(exPane("PaneFileType", 50, _("File Type")));
  panes.push_back(exPane("PaneLines", 100, _("Lines")));

  // Add the lexer pane only if we have lexers.
  if (!exApp::GetLexers()->Get().empty())
  {
#ifdef __WXMSW__
    const int lexer_size = 60;
#else
    const int lexer_size = 75;
#endif  
    panes.push_back(exPane("PaneLexer", lexer_size, _("Lexer")));
  }

  panes.push_back(exPane("PaneItems", 65, _("Items")));
  SetupStatusBar(panes);

  wxMenuBar* menubar = new wxMenuBar(wxMB_DOCKABLE); // wxMB_DOCKABLE only used for GTK
  SetMenuBar(menubar);

  exMenu *menuFile = new exMenu();
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
  menuFile->Append(ID_ALL_STC_SAVE, _("Save A&ll"), wxEmptyString, wxITEM_NORMAL, NULL, wxART_FILE_SAVE);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->Append(ID_ALL_STC_PRINT, exEllipsed(_("Print A&ll")), wxEmptyString, wxITEM_NORMAL, NULL, wxART_PRINT);
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);
  
  exMenu *menuEdit = new exMenu();
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
  menuEdit->Append(ID_SPECIAL_FIND_IN_FILES, exEllipsed(_("Find &In Files")));
  menuEdit->AppendSeparator();
  menuEdit->AppendTools(ID_STC_TOOL_MENU);
  menuEdit->AppendSeparator();
  menuEdit->Append(ID_EDIT_GOTO, exEllipsed(_("&Goto"), "Ctrl+G"));
  menuEdit->AppendSeparator();
  menuEdit->Append(ID_EDIT_CONTROL_CHAR, exEllipsed(_("&Control Char"), "Ctrl+H"));
  menuEdit->AppendSeparator();
  
  if (exApp::GetConfigBool("RCS/Local"))
  {
    menuEdit->Append(ID_COMMIT, exEllipsed(_("C&ommit")));
    menuEdit->AppendSeparator();
  }

  menuEdit->Append(ID_EDIT_MACRO_START_RECORD, _("Start Record"));
  menuEdit->Append(ID_EDIT_MACRO_STOP_RECORD, _("Stop Record"));
  menuEdit->Append(ID_EDIT_MACRO_PLAYBACK, _("Playback\tCtrl+M"));
  
  exMenu *menuView = new exMenu;
  menuView->Append(ID_VIEW_STATUSBAR, _("&Statusbar"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_TOOLBAR, _("&Toolbar"), wxEmptyString, wxITEM_CHECK);
  menuView->AppendSeparator();
  menuView->Append(ID_VIEW_FILES, _("&Files"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_PROJECTS, _("&Projects"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_DIRCTRL, _("&Explorer"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_OUTPUT, _("&Output"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_ASCII_TABLE, _("&Ascii Table"), wxEmptyString, wxITEM_CHECK);
  menuView->Append(ID_VIEW_HISTORY, _("&History"), wxEmptyString, wxITEM_CHECK);
#ifdef __WXMSW__
  ///  \todo Listctrl under GTK and X11 behave differently, items become non-existing by the OnIdle.
  wxMenu *menuListView = new wxMenu;
  menuListView->Append(wxID_VIEW_LIST, _("&List"), wxEmptyString, wxITEM_CHECK);
  menuListView->Append(wxID_VIEW_DETAILS, _("&Detail"), wxEmptyString, wxITEM_CHECK);
  menuListView->Append(wxID_VIEW_SMALLICONS, _("&Small Icon"), wxEmptyString, wxITEM_CHECK);
  menuView->AppendSeparator();
  menuView->Append(ID_VIEW_MENU, _("&Lists"), wxEmptyString, wxITEM_NORMAL, menuListView);
#endif
  
  wxMenu *menuProcess = new wxMenu();
  menuProcess->Append(ID_PROCESS_SELECT, exEllipsed(_("&Select")));
  menuProcess->AppendSeparator();
  menuProcess->Append(ID_PROCESS_RUN, _("&Run"));
  menuProcess->Append(wxID_STOP);
  
  exMenu *menuProject = new exMenu();
  menuProject->Append(ID_PROJECT_NEW, wxGetStockLabel(wxID_NEW), wxEmptyString, wxITEM_NORMAL, NULL, wxART_NEW);
  menuProject->Append(ID_PROJECT_OPEN, wxGetStockLabel(wxID_OPEN), wxEmptyString, wxITEM_NORMAL, NULL, wxART_FILE_OPEN);
  UseProjectHistory(ID_RECENT_PROJECT_MENU, menuProject);
  menuProject->Append(ID_PROJECT_OPENTEXT, _("&Open As Text"));
  menuProject->Append(ID_PROJECT_CLOSE, wxGetStockLabel(wxID_CLOSE));
  menuProject->AppendSeparator();
  menuProject->Append(ID_PROJECT_SAVE, wxGetStockLabel(wxID_SAVE), wxEmptyString, wxITEM_NORMAL, NULL, wxART_FILE_SAVE);
  menuProject->Append(ID_PROJECT_SAVEAS, wxGetStockLabel(wxID_SAVEAS), wxEmptyString, wxITEM_NORMAL, NULL, wxART_FILE_SAVE_AS);
  menuProject->AppendSeparator();
  menuProject->Append(ID_SORT_SYNC, _("&Auto Sort"), wxEmptyString, wxITEM_CHECK);
  
  wxMenu *menuWindow = new wxMenu();
  menuWindow->Append(ID_SPLIT, _("Split"));
  
  wxMenu* menuOptions = new wxMenu();
  menuOptions->Append(ID_OPTION_COMPARATOR, exEllipsed(_("Set &Comparator")));
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_LIST_COLOUR, exEllipsed(_("Set &List Colour")));
  menuOptions->Append(ID_OPTION_LIST_FONT, exEllipsed(_("Set &List Font")));
  wxMenu *menuListSort = new wxMenu;
  menuListSort->Append(ID_OPTION_LIST_SORT_ASCENDING, _("&Ascending"), wxEmptyString, wxITEM_CHECK);
  menuListSort->Append(ID_OPTION_LIST_SORT_DESCENDING, _("&Descending"), wxEmptyString, wxITEM_CHECK);
  menuListSort->Append(ID_OPTION_LIST_SORT_TOGGLE, _("&Toggle"), wxEmptyString, wxITEM_CHECK);
  menuOptions->Append(ID_OPTION_LIST_SORT_MENU, _("Set &List Sort Method"), menuListSort);
  menuOptions->AppendSeparator();
  menuOptions->Append(ID_OPTION_EDITOR, exEllipsed(_("Set &Editor Options")));
  wxMenu *menuHelp = new wxMenu();
  // Both wxART_HELP_BOOK, and wxART_HELP_PAGE do not fit nicely on menu item.
  menuHelp->Append(wxID_HELP_CONTENTS, _("&Contents"));
  menuHelp->AppendSeparator();
  menuHelp->Append(wxID_ABOUT);
  menubar->Append(menuFile, _("&File"));
  menubar->Append(menuEdit, _("&Edit"));
  menubar->Append(menuView, _("&View"));
  menubar->Append(menuProcess, _("&Process"));
  menubar->Append(menuProject, _("&Project"));
  menubar->Append(menuWindow, _("&Window"));
  menubar->Append(menuOptions, _("&Options"));
  menubar->Append(menuHelp, wxGetStockLabel(wxID_HELP));

  m_ToolBar = new exToolBar(this, 
    wxID_ANY,
    wxDefaultPosition, 
    wxDefaultSize, 
    wxNO_BORDER | wxTB_FLAT | wxTB_NODIVIDER);

  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->AddTool(wxID_SAVE);
  m_ToolBar->AddTool(wxID_PRINT);
  m_ToolBar->AddSeparator();
  m_ToolBar->AddTool(wxID_FIND);
#ifdef __WXMSW__
  const wxSize tbz(150, 20);
#else
  const wxSize tbz(150, -1);
#endif  
  m_ToolBar->AddControl(new ftFind(m_ToolBar, this, ID_FIND_TEXT, wxDefaultPosition, tbz));
  
  m_ToolBar->AddSeparator();
  m_ToolBar->AddTool(
    ID_PROJECT_OPEN, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, m_ToolBar->GetToolBitmapSize()), 
    _("Open project..."));
  m_ToolBar->AddTool(
    ID_PROJECT_SAVE, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, m_ToolBar->GetToolBitmapSize()), 
    _("Save project"));
#ifdef __WXMSW__
  ///  \todo See comment above.
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
  
#endif
#if wxUSE_CHECKBOX
  m_ToolBar->AddSeparator();
#ifndef __WXMSW__
  wxSize size(55, 25);
#else
  wxSize size = wxDefaultSize;
#endif

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
  m_HexModeCheckBox->SetValue(exApp::GetConfigBool("HexMode"));
  m_SyncCheckBox->SetToolTip(_("Synchronize modified files"));
#endif // wxUSE_CHECKBOX

  m_ToolBar->Realize();
}

bool Frame::AllowClose(wxWindowID id, wxWindow* page) 
{
  if (ftListView::ProcessIsRunning())
    return false;
  else if (id == NOTEBOOK_EDITORS)
    return ((ftSTC*)page)->Continue();
  else if (id == NOTEBOOK_PROJECTS)
    return ((ftListView*)page)->Continue();
  else
    return ftFrame::AllowClose(id, page);
}

void Frame::OnNotebook(wxWindowID id, wxWindow* page) 
{
  if (id == NOTEBOOK_EDITORS)
  {
    ((ftSTC*)page)->PropertiesMessage();
  }
  else if (id == NOTEBOOK_PROJECTS)
  {
    SetTitle(wxEmptyString, ((ftListView*)page)->GetFileName().GetName());
    exStatusText(((ftListView*)page)->GetFileName());
  }
}
