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

#include <wx/aboutdlg.h>
#include <wx/stdpaths.h> // for wxStandardPaths
#include <wx/extension/report/dir.h>
#include "appl.h"

#ifndef __WXMSW__
#include "appl.xpm"
#endif

enum
{
  ID_PROCESS_DIALOG,
  ID_PROCESS_RUN,
  ID_RECENTFILE_MENU,
};

BEGIN_EVENT_TABLE(wxExRepSampleFrame, wxExFrameWithHistory)
  EVT_MENU(wxID_STOP, wxExRepSampleFrame::OnCommand)
  EVT_MENU(ID_PROCESS_DIALOG, wxExRepSampleFrame::OnCommand)
  EVT_MENU(ID_PROCESS_RUN, wxExRepSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExRepSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_OPEN, wxID_PREFERENCES, wxExRepSampleFrame::OnCommand)
  EVT_TREE_ITEM_ACTIVATED(wxID_TREECTRL, wxExRepSampleFrame::OnTree)
END_EVENT_TABLE()

IMPLEMENT_APP(wxExRepSampleApp)

bool wxExRepSampleApp::OnInit()
{
  SetAppName("wxExRepSample");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  SetLogging();

  wxExRepSampleFrame *frame = new wxExRepSampleFrame();

  frame->Show(true);

  SetTopWindow(frame);

  return true;
}

wxExRepSampleFrame::wxExRepSampleFrame()
  : wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppName())
{
  SetIcon(wxICON(appl));

  wxExMenu *menuFile = new wxExMenu;
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu *menuProcess = new wxExMenu;
  menuProcess->Append(ID_PROCESS_DIALOG, wxExEllipsed(_("Dialog")));
  menuProcess->Append(ID_PROCESS_RUN, _("Run"));
  menuProcess->AppendSeparator();
  menuProcess->Append(wxID_STOP);

  wxExMenu* menuHelp = new wxExMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, _("&File"));
  menubar->Append(menuProcess, _("&Process"));
  menubar->Append(menuHelp, _("&Help"));
  SetMenuBar(menubar);

  CreateToolBar();

  m_ToolBar->AddTool(wxID_OPEN);
  m_ToolBar->Realize();

#if wxUSE_STATUSBAR
  std::vector<wxExPane> panes;
  panes.push_back(wxExPane("PaneText", -3));
  panes.push_back(wxExPane("PaneFileType", 50));
  panes.push_back(wxExPane("PaneLines", 100));
  panes.push_back(wxExPane("PaneLexer", 60));
  panes.push_back(wxExPane("PaneItems", 60));
  SetupStatusBar(panes);
#endif

  const wxExLexer lexer = wxExLexers::Get()->FindByName("cpp");

  m_DirCtrl = new wxGenericDirCtrl(this, wxID_ANY, wxStandardPaths::Get().GetDocumentsDir());

  m_NotebookWithLists = new wxExNotebook(
    this, this,
    wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_WINDOWLIST_BUTTON);

  m_STC = new wxExSTCWithFrame(this); // use all flags (default)

  for (
    int i = wxExListViewWithFrame::LIST_BEFORE_FIRST + 1;
    i < wxExListViewWithFrame::LIST_AFTER_LAST;
    i++)
  {
    wxExListViewWithFrame* vw = new wxExListViewWithFrame(
      this, 
      (wxExListViewWithFrame::ListType)i, 
      wxID_ANY,
      0xFF, 
      &lexer); // set all flags

    m_NotebookWithLists->AddPage(vw, vw->GetTypeDescription(), vw->GetTypeDescription(), true);
  }

  GetManager().AddPane(m_STC, wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));
  GetManager().AddPane(m_NotebookWithLists, wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250)));
  GetManager().AddPane(m_DirCtrl, wxAuiPaneInfo().Caption(_("DirCtrl")).Left().MinSize(wxSize(250, 250)));
  GetManager().AddPane(new wxExFindToolBar(this, this),
    wxAuiPaneInfo().ToolbarPane().Bottom().Name("FINDBAR").Caption(_("Findbar")));

  GetManager().Update();

  wxExDirWithListView dir(
    (wxExListViewWithFrame*)m_NotebookWithLists->GetPageByKey(
      wxExListViewWithFrame::GetTypeDescription(wxExListViewWithFrame::LIST_PROJECT)),
    wxGetCwd(),
    "*.cpp;*.h");

  dir.FindFiles();

  wxExListItemWithFileName item(
    (wxExListViewWithFrame*)m_NotebookWithLists->GetPageByKey(
      wxExListViewWithFrame::GetTypeDescription(wxExListViewWithFrame::LIST_PROJECT)),
    "NOT EXISTING ITEM");

  item.Insert();
}

wxExListViewWithFrame* wxExRepSampleFrame::Activate(
  wxExListViewWithFrame::ListType type, 
  const wxExLexer* lexer)
{
  for (
    size_t i = 0;
    i < m_NotebookWithLists->GetPageCount();
    i++)
  {
    wxExListViewWithFrame* vw = (wxExListViewWithFrame*)m_NotebookWithLists->GetPage(i);

    if (vw->GetType() == type)
    {
      if (type == wxExListViewWithFrame::LIST_KEYWORD)
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

wxExListView* wxExRepSampleFrame::GetListView()
{
  return (wxExListView*)m_NotebookWithLists->GetPage(
    m_NotebookWithLists->GetSelection());
}


wxExSTC* wxExRepSampleFrame::GetSTC()
{
  return m_STC;
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
  
  case wxID_OPEN:
    event.Skip();
    break;
    
  case wxID_PREVIEW:
    if (m_STC->HasCapture())
    {
      m_STC->PrintPreview();
    }
    else
    {
      wxExListView* lv = GetFocusedListView();

      if (lv != NULL)
      {
        lv->PrintPreview();
      }
    }
    break;
    
  case wxID_PRINT:
    {
      wxExListView* lv = GetFocusedListView();

      if (lv != NULL)
      {
        lv->Print();
      }
    }
    break;
    
  case wxID_PRINT_SETUP: wxExApp::GetPrinter()->PageSetup(); 
    break;

  case wxID_STOP:
    if (ProcessIsRunning())
    {
      ProcessStop();
    }
    break;

  case ID_PROCESS_DIALOG:
    wxExProcessWithListView::ConfigDialog(this);
    break;

  case ID_PROCESS_RUN:
    ProcessRun();
    break;

  default: 
    wxFAIL;
    break;
  }
}

void wxExRepSampleFrame::OnTree(wxTreeEvent& /* event */)
{
  const wxString selection = m_DirCtrl->GetFilePath();

  if (!selection.empty())
  {
    OpenFile(wxExFileName(selection));
  }
}

bool wxExRepSampleFrame::OpenFile(const wxExFileName& file,
  int line_number,
  const wxString& match,
  long flags)
{
  // We cannot use the wxExFrameWithHistory::OpenFile, as that uses the focused STC.
  // Prevent recursion.
  if ((flags & wxExSTC::STC_OPEN_FROM_LINK) |
      (flags & wxExSTC::STC_OPEN_FROM_STATISTICS))
  {
    flags = 0;
  }

  return m_STC->Open(file, line_number, match, flags);
}
