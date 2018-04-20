////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of sample classes for wxExRep
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wx/extension/lexers.h>
#include <wx/extension/listitem.h>
#include <wx/extension/log.h>
#include <wx/extension/printing.h>
#include <wx/extension/toolbar.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/dirctrl.h>
#include <wx/extension/report/listview.h>
#include <easylogging++.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

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

wxExRepSampleFrame::wxExRepSampleFrame() : wxExFrameWithHistory()
{
  SetIcon(wxICON(app));

  wxExMenu *menuFile = new wxExMenu;
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wxExMenu *menuView = new wxExMenu;
  AppendPanes(menuView);

  wxExMenu* menuHelp = new wxExMenu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuView, "&View");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

  GetToolBar()->AddControls();
  
#if wxUSE_STATUSBAR
  SetupStatusBar({
    {"PaneFileType", 50},
    {"PaneInfo", 100},
    {"PaneLexer", 60}});
#endif

  m_NotebookWithLists = new wxExNotebook(
    wxExWindowData().Style(wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON));

  m_STC = new wxExSTC();

  const wxExLexer lexer = wxExLexers::Get()->FindByName("cpp");

  for (int i = LIST_FOLDER; i <= LIST_FILE; i++)
  {
    auto* vw = new wxExListViewWithFrame(
      wxExListViewData().Type((wxExListType)i).Lexer(&lexer));

    m_NotebookWithLists->AddPage(
      vw, 
      vw->GetData().TypeDescription(), 
      vw->GetData().TypeDescription(), 
      true);
  }

  GetManager().AddPane(
    m_STC, 
    wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));

  GetManager().AddPane(
    m_NotebookWithLists, 
    wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250)));

  GetManager().AddPane(
    new wxExGenericDirCtrl(this),
    wxAuiPaneInfo().Caption("DirCtrl").Left().MinSize(wxSize(250, 250)));

  GetManager().Update();

  wxExDirWithListView dir(
    (wxExListView*)m_NotebookWithLists->GetPageByKey(
      wxExListViewData().Type(LIST_FILE).TypeDescription()),
    wxExPath::Current(),
    "*.cpp;*.h");

  dir.FindFiles();

  wxExListItem item(
    (wxExListView*)m_NotebookWithLists->GetPageByKey(
      wxExListViewData().Type(LIST_FILE).TypeDescription()),
    wxExPath("NOT EXISTING ITEM"));

  item.Insert();
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wxExGetVersionInfo().GetVersionOnlyString());
    info.SetCopyright(wxExGetVersionInfo().GetCopyright());
    wxAboutBox(info);}, wxID_ABOUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {;}, wxID_HELP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_STC->GetFile().FileNew(wxExPath());}, wxID_NEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_STC->HasCapture())
    {
      m_STC->PrintPreview();
    }
    else
    {
      auto* lv = GetListView();

      if (lv != nullptr)
      {
        lv->PrintPreview();
      }
      else
      {
        wxLogStatus("No focus");
      }
    }}, wxID_PREVIEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    auto* lv = GetListView();

    if (lv != nullptr)
    {
      lv->Print();
    }
    else
    {
      wxLogStatus("No focus");
    }}, wxID_PRINT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
	  wxExPrinting::Get()->GetHtmlPrinter()->PageSetup();}, wxID_PRINT_SETUP);
}

wxExListView* wxExRepSampleFrame::Activate(
  wxExListType type, 
  const wxExLexer* lexer)
{
  for (
    size_t i = 0;
    i < m_NotebookWithLists->GetPageCount();
    i++)
  {
    wxExListView* vw = (wxExListView*)m_NotebookWithLists->GetPage(i);

    if (vw->GetData().Type() == type)
    {
      if (type == LIST_KEYWORD)
      {
        if (lexer != nullptr)
        {
          if (lexer->GetScintillaLexer() != "cpp")
          {
            if (!lexer->GetDisplayLexer().empty())
            {
              VLOG(9) << lexer->GetDisplayLexer() << ", only cpp for the sample";
            }
              
            return nullptr;
          }
        }
      }

      return vw;
    }
  }

  return nullptr;
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
  
wxExSTC* wxExRepSampleFrame::OpenFile(const wxExPath& file, const wxExSTCData& data)
{
  m_STC->GetLexer().Reset();

  // We cannot use the wxExFrameWithHistory::OpenFile, as that uses GetSTC.
  // Prevent recursion.
  m_STC->Open(file, wxExSTCData(data).Flags(STC_WIN_DEFAULT));
  
  return m_STC;
}
