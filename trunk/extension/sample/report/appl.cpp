/******************************************************************************\
* File:          appl.cpp
* Purpose:       Implementation of sample classes for wxExRep
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef __WXMSW__
  #include "mondrian.xpm"
#endif

#include <wx/aboutdlg.h>
#include <wx/stdpaths.h> // for wxStandardPaths
#include <wx/extension/report/report.h>
#include "appl.h"

enum
{
  ID_PROCESS_DIALOG,
  ID_PROCESS_RUN,
  ID_RECENTFILE_MENU,
};

BEGIN_EVENT_TABLE(wxExRepSampleFrame, exFrameWithHistory)
  EVT_MENU(ID_PROCESS_DIALOG, wxExRepSampleFrame::OnCommand)
  EVT_MENU(ID_PROCESS_RUN, wxExRepSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_LOWEST, wxID_HIGHEST, wxExRepSampleFrame::OnCommand)
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, wxExRepSampleFrame::OnTree)
END_EVENT_TABLE()

IMPLEMENT_APP(wxExRepSampleApp)

bool wxExRepSampleApp::OnInit()
{
  SetAppName("wxExRepSample");
  SetLogging();

  exApp::OnInit();

  wxExRepSampleFrame *frame = new wxExRepSampleFrame("wxExRepSample");

  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

wxExRepSampleFrame::wxExRepSampleFrame(const wxString& title)
  : exFrameWithHistory(NULL, wxID_ANY, title)
{
  SetIcon(wxICON(mondrian));

  exMenu *menuFile = new exMenu;
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  exMenu *menuProcess = new exMenu;
  menuProcess->Append(ID_PROCESS_DIALOG, exEllipsed(_("Dialog")));
  menuProcess->Append(ID_PROCESS_RUN, _("Run"));
  menuProcess->AppendSeparator();
  menuProcess->Append(wxID_STOP);
  
  exMenu* menuHelp = new exMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, _("&File"));
  menubar->Append(menuProcess, _("&Process"));
  menubar->Append(menuHelp, _("&Help"));
  SetMenuBar(menubar);

  CreateToolBar();

  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->Realize();

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

  m_STC = new exSTCWithFrame(this); // use all flags (default)

  for (
    int i = exListViewFile::LIST_BEFORE_FIRST + 1;
    i < exListViewFile::LIST_AFTER_LAST;
    i++)
  {
    exListViewFile* vw = new exListViewFile(this, (exListViewFile::ListType)i, 0xFF, &lexer); // set all flags
    m_NotebookWithLists->AddPage(vw, vw->GetTypeDescription(), vw->GetTypeDescription(), true);
  }

  GetManager().AddPane(m_STC, wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));
  GetManager().AddPane(m_NotebookWithLists, wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_DirCtrl, wxAuiPaneInfo().Caption(_("DirCtrl")).Left().MinSize(wxSize(250, 250)));
  GetManager().AddPane(new exFindToolBar(this, this),
    wxAuiPaneInfo().ToolbarPane().Bottom().Name("FINDBAR").Caption(_("Findbar")));

  GetManager().Update();

  exDirWithReport dir(
    (exListViewFile*)m_NotebookWithLists->GetPageByKey(
      exListViewFile::GetTypeDescription(exListViewFile::LIST_PROJECT)),
    wxGetCwd(), 
    "*.cpp;*.h");

  dir.FindFiles();

  exListItemWithFileName item(
    (exListViewFile*)m_NotebookWithLists->GetPageByKey(
      exListViewFile::GetTypeDescription(exListViewFile::LIST_PROJECT)), 
    "NOT EXISTING ITEM");

  item.Insert();
}

exListViewFile* wxExRepSampleFrame::Activate(int type, const exLexer* lexer)
{
  for (
    size_t i = 0;
    i < m_NotebookWithLists->GetPageCount();
    i++)
  {
    exListViewFile* vw = (exListViewFile*)m_NotebookWithLists->GetPage(i);

    if (vw->GetType() == type)
    {
      if (type == exListViewFile::LIST_KEYWORD)
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

void wxExRepSampleFrame::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_ABOUT:
    {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wxEX_VERSION_STRING);
    info.AddDeveloper(wxVERSION_STRING);
    info.SetCopyright(_("(c) 1998-2009 Anton van Wezenbeek."));
    wxAboutBox(info);
    }
    break;
  case wxID_EXIT: Close(true); break;
  case wxID_PREVIEW: 
    if (m_STC->HasCapture())
    {
      m_STC->PrintPreview();
    }
    else
    {
      exListViewFile* lv = GetFocusedListView();

      if (lv != NULL)
      {
        lv->PrintPreview(); 
      }
    }
    break;
  case wxID_PRINT: 
    {
      exListViewFile* lv = GetFocusedListView();

      if (lv != NULL)
      {
        lv->Print(); 
      }
    }
    break;
  case wxID_PRINT_SETUP: exApp::GetPrinter()->PageSetup(); break;

  case wxID_STOP:
    if (exListViewFile::ProcessIsRunning())
    {
      exListViewFile::ProcessStop();
    }
    break;

  case ID_PROCESS_DIALOG:
    exProcessWithListView::ConfigDialog(); 
    break;

  case ID_PROCESS_RUN:
    exListViewFile::ProcessRun(); 
    break;

  default: event.Skip();
  }
}

void wxExRepSampleFrame::OnTree(wxTreeEvent& event)
{
  const wxString selection = m_DirCtrl->GetFilePath();

  if (!selection.empty())
  {
    OpenFile(exFileName(selection));
  }
}

bool wxExRepSampleFrame::OpenFile(const exFileName& file, 
  int line_number, 
  const wxString& match,
  long flags)
{
  // We cannot use the exFrameWithHistory::OpenFile, as that uses the focused STC.
  // Prevent recursion.
  if ((flags & exSTC::STC_OPEN_FROM_LINK) |
      (flags & exSTC::STC_OPEN_FROM_STATISTICS))
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
