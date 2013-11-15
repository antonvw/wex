////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of sample classes for wxExRep
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/listitem.h>
#include <wx/extension/printing.h>
#include <wx/extension/process.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

enum
{
  ID_PROCESS_DIALOG,
  ID_PROCESS_RUN,
  ID_RECENTFILE_MENU
};

BEGIN_EVENT_TABLE(wxExRepSampleFrame, wxExFrameWithHistory)
  EVT_MENU(wxID_STOP, wxExRepSampleFrame::OnCommand)
  EVT_MENU(ID_PROCESS_DIALOG, wxExRepSampleFrame::OnCommand)
  EVT_MENU(ID_PROCESS_RUN, wxExRepSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExRepSampleFrame::OnCommand)
  EVT_MENU_RANGE(wxID_OPEN, wxID_PREFERENCES, wxExRepSampleFrame::OnCommand)
END_EVENT_TABLE()

wxIMPLEMENT_APP(wxExRepSampleApp);

bool wxExRepSampleApp::OnInit()
{
  SetAppName("wxex-sample-rep");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  wxExRepSampleFrame *frame = new wxExRepSampleFrame();
  frame->Show(true);

  return true;
}

wxExRepSampleFrame::wxExRepSampleFrame()
  : wxExFrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppDisplayName())
  , m_Process(new wxExProcess())
{
  SetIcon(wxICON(app));

  wxExMenu *menuFile = new wxExMenu;
  menuFile->Append(wxID_OPEN);
  UseFileHistory(ID_RECENTFILE_MENU, menuFile);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu *menuProcess = new wxExMenu;
  menuProcess->Append(ID_PROCESS_DIALOG, wxExEllipsed("Dialog"));
  menuProcess->Append(ID_PROCESS_RUN, "Run");
  menuProcess->AppendSeparator();
  menuProcess->Append(wxID_STOP);

  wxExMenu *menuView = new wxExMenu;
  menuView->AppendBars();

  wxExMenu* menuHelp = new wxExMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuView, "&View");
  menubar->Append(menuProcess, "&Process");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

#if wxUSE_STATUSBAR
  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("PaneFileType", 50));
  panes.push_back(wxExStatusBarPane("PaneInfo", 100));
  panes.push_back(wxExStatusBarPane("PaneLexer", 60));
  SetupStatusBar(panes);
#endif

  m_NotebookWithLists = new wxExNotebook(
    this, this,
    wxID_ANY, wxDefaultPosition, wxDefaultSize,
    wxAUI_NB_DEFAULT_STYLE |
    wxAUI_NB_WINDOWLIST_BUTTON);

  m_STC = new wxExSTCWithFrame(this, this); // use all flags (default)

  const wxExLexer lexer = wxExLexers::Get()->FindByName("cpp");

  for (
    int i = wxExListViewFileName::LIST_BEFORE_FIRST + 1;
    i < wxExListViewFileName::LIST_AFTER_LAST;
    i++)
  {
    wxExListViewWithFrame* vw = new wxExListViewWithFrame(
      this,
      this, 
      (wxExListViewFileName::wxExListType)i, 
      wxID_ANY,
      0xFF, 
      &lexer); // set all flags

    m_NotebookWithLists->AddPage(
      vw, 
      vw->GetTypeDescription(), 
      vw->GetTypeDescription(), 
      true);
  }

  GetManager().AddPane(
    m_STC, 
    wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));

  GetManager().AddPane(
    m_NotebookWithLists, 
    wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250)));

  GetManager().AddPane(
    new wxExGenericDirCtrl(this, this),
    wxAuiPaneInfo().Caption("DirCtrl").Left().MinSize(wxSize(250, 250)));

  GetManager().Update();

  wxExDirWithListView dir(
    (wxExListView*)m_NotebookWithLists->GetPageByKey(
      wxExListViewFileName::GetTypeDescription(wxExListViewFileName::LIST_FILE)),
    wxGetCwd(),
    "*.cpp;*.h");

  dir.FindFiles();

  wxExListItem item(
    (wxExListView*)m_NotebookWithLists->GetPageByKey(
      wxExListViewFileName::GetTypeDescription(wxExListViewFileName::LIST_FILE)),
    wxFileName("NOT EXISTING ITEM"));

  item.Insert();
}

wxExListViewFileName* wxExRepSampleFrame::Activate(
  wxExListViewFileName::wxExListType type, 
  const wxExLexer* lexer)
{
  for (
    size_t i = 0;
    i < m_NotebookWithLists->GetPageCount();
    i++)
  {
    wxExListViewFileName* vw = (wxExListViewFileName*)m_NotebookWithLists->GetPage(i);

    if (vw->GetType() == type)
    {
      if (type == wxExListViewFileName::LIST_KEYWORD)
      {
        if (lexer != NULL)
        {
          if (lexer->GetScintillaLexer() != "cpp")
          {
            if (!lexer->GetDisplayLexer().empty())
            {
              wxLogMessage(lexer->GetDisplayLexer() + ", only cpp for the sample");
            }
              
            return NULL;
          }
        }
      }

      return vw;
    }
  }

  return NULL;
}

bool wxExRepSampleFrame::AllowClose(wxWindowID id, wxWindow* page)
{
  if (page == GetFileHistoryList())
  {
    // prevent possible crash, if SetRecentFile tries
    // to add listitem to deleted history list.
    return false;
  }  
  else
  {
    return wxExFrameWithHistory::AllowClose(id, page);
  }
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
    info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());
    info.SetCopyright(wxExGetVersionInfo().GetCopyright());
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
      auto* lv = GetListView();

      if (lv != NULL)
      {
        lv->PrintPreview();
      }
      else
      {
        wxLogStatus("No focus");
      }
    }
    break;
    
  case wxID_PRINT:
    {
      auto* lv = GetListView();

      if (lv != NULL)
      {
        lv->Print();
      }
      else
      {
        wxLogStatus("No focus");
      }
    }
    break;
    
  case wxID_PRINT_SETUP:
	  wxExPrinting::Get()->GetHtmlPrinter()->PageSetup();
    break;

  case wxID_STOP:
    m_Process->Kill();
    break;

  case ID_PROCESS_DIALOG:
    wxExProcess::ConfigDialog(this);
    break;

  case ID_PROCESS_RUN:
    m_Process->Execute();
    break;

  default: 
    wxFAIL;
    break;
  }
}

bool wxExRepSampleFrame::OpenFile(const wxExFileName& file,
  int line_number,
  const wxString& match,
  long flags)
{
  // We cannot use the wxExFrameWithHistory::OpenFile, as that uses GetSTC.
  // Prevent recursion.
  if (flags & wxExSTC::STC_WIN_FROM_OTHER)
  {
    flags = 0;
  }

  return m_STC->Open(file, line_number, match, flags);
}
