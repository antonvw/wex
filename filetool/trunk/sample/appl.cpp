/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of sample classes for wxFileTool
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef __WXMSW__
  #include "mondrian.xpm"
#endif

#include <wx/stdpaths.h> // for wxStandardPaths
#include <wx/filetool/filetool.h>
#include <wx/filetool/process.h>
#include "appl.h"

enum
{
  ID_PROCESS_DIALOG,
  ID_PROCESS_RUN,
  ID_RECENTFILE_MENU,
};

BEGIN_EVENT_TABLE(ftSampleFrame, ftFrame)
  EVT_MENU(ID_PROCESS_DIALOG, ftSampleFrame::OnCommand)
  EVT_MENU(ID_PROCESS_RUN, ftSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_LOWEST, wxID_HIGHEST, ftSampleFrame::OnCommand)
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, ftSampleFrame::OnTree)
END_EVENT_TABLE()

IMPLEMENT_APP(ftSampleApp)

bool ftSampleApp::OnInit()
{
  SetAppName("ftSample");
  SetLogging();

  exApp::OnInit();

  ftSampleFrame *frame = new ftSampleFrame("ftSample");

  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

ftSampleFrame::ftSampleFrame(const wxString& title)
  : ftFrame(NULL, wxID_ANY, title)
{
  SetIcon(wxICON(mondrian));

  exMenu *menuFile = new exMenu;
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->Append(ID_PROCESS_DIALOG, _("Process Dialog"));
  menuFile->Append(ID_PROCESS_RUN, _("Process Run"));
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, _("&File"));
  SetMenuBar(menubar);

  wxToolBar* toolBar = new wxToolBar(this, 
    wxID_ANY, wxDefaultPosition, wxDefaultSize, wxNO_BORDER | wxTB_FLAT | wxTB_NODIVIDER);
  SetToolBar(toolBar);

  const wxSize toolbar_size(16, 15);
  toolBar->SetToolBitmapSize(toolbar_size);
  toolBar->AddTool(wxID_OPEN, wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, toolbar_size), _("Open file"));
  toolBar->AddControl(new ftFind(toolBar, this));
  toolBar->Realize();

  std::vector<exPane> panes;
  panes.push_back(exPane("PaneText", -3));
  panes.push_back(exPane("PaneFileType", 50));
  panes.push_back(exPane("PaneLines", 100));
  panes.push_back(exPane("PaneLexer", 60));
  panes.push_back(exPane("PaneItems", 60));
  SetupStatusBar(panes);

  const exLexer lexer = exApp::GetLexers()->FindByName("cpp");

  m_DirCtrl = new wxGenericDirCtrl(this, wxID_ANY, wxStandardPaths::Get().GetDocumentsDir());

  m_NotebookWithLists = new exNotebook(
    this, this,
    wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxAUI_NB_DEFAULT_STYLE | 
    wxAUI_NB_WINDOWLIST_BUTTON);

  m_STC = new ftSTC(this); // use all flags (default)

  for (
    int i = ftListView::LIST_BEFORE_FIRST + 1;
    i < ftListView::LIST_AFTER_LAST;
    i++)
  {
    ftListView* vw = new ftListView(this, (ftListView::ftListType)i, 0xFF, &lexer); // set all flags
    m_NotebookWithLists->AddPage(vw, vw->GetTypeDescription(), vw->GetTypeDescription(), true);
  }

  GetManager().AddPane(m_STC, wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));
  GetManager().AddPane(m_NotebookWithLists, wxAuiPaneInfo().CloseButton(false).MaximizeButton(true));
  GetManager().AddPane(m_DirCtrl, wxAuiPaneInfo().Caption(_("DirCtrl")));

  GetManager().Update();

  ftDir dir(
    (ftListView*)m_NotebookWithLists->GetPageByKey(
      ftListView::GetTypeDescription(ftListView::LIST_PROJECT)),
    wxGetCwd(), 
    "*.cpp;*.h");

  dir.FindFiles();

  ftListItem item(
    (ftListView*)m_NotebookWithLists->GetPageByKey(
      ftListView::GetTypeDescription(ftListView::LIST_PROJECT)), 
    "NOT EXISTING ITEM");

  item.Insert();
}

ftListView* ftSampleFrame::Activate(int type, const exLexer* lexer)
{
  for (
    std::map<wxString, wxWindow*>::const_iterator it = m_NotebookWithLists->GetMapPages().begin();
    it != m_NotebookWithLists->GetMapPages().end();
    ++it)
  {
    ftListView* vw = (ftListView*)it->second;

    if (vw->GetType() == type)
    {
      if (type == ftListView::LIST_KEYWORD)
      {
        if (lexer != NULL)
        {
          if (lexer->GetScintillaLexer() != "cpp")
          {
            wxLogMessage(lexer->GetScintillaLexer() + ", only cpp for the sample");
            return NULL;
          }
        }
      }

      return vw;
    }
  }

  return NULL;
}

void ftSampleFrame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_EXIT: Close(true); break;
  case wxID_PREVIEW: 
    if (m_STC->HasCapture())
    {
      m_STC->PrintPreview();
    }
    else
    {
      ftListView* lv = GetFocusedListView();

      if (lv != NULL)
      {
        lv->PrintPreview(); 
      }
    }
    break;
  case wxID_PRINT: 
    {
      ftListView* lv = GetFocusedListView();

      if (lv != NULL)
      {
        lv->Print(); 
      }
    }
    break;
  case wxID_PRINT_SETUP: exApp::GetPrinter()->PageSetup(); break;

  case ID_PROCESS_DIALOG:
    ftProcess::ConfigDialog(); 
    break;

  case ID_PROCESS_RUN:
    ftListView::ProcessRun(); 
    break;

  default: event.Skip();
  }
}

void ftSampleFrame::OnTree(wxTreeEvent& event)
{
  const wxString selection = m_DirCtrl->GetFilePath();

  if (!selection.empty())
  {
    OpenFile(selection);
  }
}

bool ftSampleFrame::OpenFile(const wxString& file, 
  int line_number, 
  const wxString& match,
  long flags)
{
  // We cannot use the ftFrame::OpenFile, as that uses the focused STC.
  // Prevent recursion.
  if (flags & exSTC::STC_OPEN_FROM_LINK |
      flags & exSTC::STC_OPEN_FROM_STATISTICS)
  {
    flags = 0;
  }

  bool result = m_STC->Open(file, line_number, match, flags);

  if (result)
  {
    m_STC->PropertiesMessage();
  }

  return result;
}
