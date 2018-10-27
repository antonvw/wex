////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex report sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/aboutdlg.h>
#include <wex/lexers.h>
#include <wex/listitem.h>
#include <wex/log.h>
#include <wex/printing.h>
#include <wex/toolbar.h>
#include <wex/util.h>
#include <wex/version.h>
#include <wex/report/dir.h>
#include <wex/report/dirctrl.h>
#include <wex/report/listview.h>
#include <easylogging++.h>
#include "app.h"
#ifndef __WXMSW__
#include "app.xpm"
#endif

wxIMPLEMENT_APP(report_sample_app);

bool report_sample_app::OnInit()
{
  SetAppName("wex-sample-rep");

  if (!wex::app::OnInit())
  {
    return false;
  }

  report_sample_frame *frame = new report_sample_frame();
  frame->Show(true);

  return true;
}

report_sample_frame::report_sample_frame() : wex::history_frame()
{
  SetIcon(wxICON(app));

  wex::menu *menuFile = new wex::menu;
  menuFile->Append(wxID_OPEN);
  menuFile->AppendSeparator();
  menuFile->AppendPrint();
  menuFile->AppendSeparator();
  menuFile->Append(wxID_EXIT);

  wex::menu *menuView = new wex::menu;
  AppendPanes(menuView);

  wex::menu* menuHelp = new wex::menu;
  menuHelp->Append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuView, "&View");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

  GetToolBar()->AddControls();
  
  SetupStatusBar({
    {"PaneFileType", 50},
    {"PaneInfo", 100},
    {"PaneLexer", 60}});

  m_NotebookWithLists = new wex::notebook(
    wex::window_data().Style(wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON));

  m_STC = new wex::stc();

  const wex::lexer lexer = wex::lexers::Get()->FindByName("cpp");

  for (int i = wex::listview_data::FOLDER; i <= wex::listview_data::FILE; i++)
  {
    auto* vw = new wex::history_listview(
      wex::listview_data().Type((wex::listview_data::type)i).Lexer(&lexer));

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
    new wex::dirctrl(this),
    wxAuiPaneInfo().Caption("DirCtrl").Left().MinSize(wxSize(250, 250)));

  GetManager().Update();

  wex::listview_dir dir(
    (wex::listview*)m_NotebookWithLists->GetPageByKey(
      wex::listview_data().Type(wex::listview_data::FILE).TypeDescription()),
    wex::path::Current(),
    "*.cpp;*.h");

  dir.FindFiles();

  wex::listitem item(
    (wex::listview*)m_NotebookWithLists->GetPageByKey(
      wex::listview_data().Type(wex::listview_data::FILE).TypeDescription()),
    wex::path("NOT EXISTING ITEM"));

  item.Insert();
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wex::get_version_info().Get());
    info.SetCopyright(wex::get_version_info().Copyright());
    wxAboutBox(info);}, wxID_ABOUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {;}, wxID_HELP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_STC->GetFile().FileNew(wex::path());}, wxID_NEW);

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
    wex::printing::Get()->GetHtmlPrinter()->PageSetup();}, wxID_PRINT_SETUP);
}

wex::listview* report_sample_frame::Activate(
  wex::listview_data::type type, 
  const wex::lexer* lexer)
{
  for (
    size_t i = 0;
    i < m_NotebookWithLists->GetPageCount();
    i++)
  {
    wex::listview* vw = (wex::listview*)m_NotebookWithLists->GetPage(i);

    if (vw->GetData().Type() == type)
    {
      if (type == wex::listview_data::KEYWORD)
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

bool report_sample_frame::AllowClose(wxWindowID id, wxWindow* page)
{
  if (page == GetFileHistoryList())
  {
    // prevent possible crash, if SetRecentFile tries
    // to add listitem to deleted history list.
    return false;
  }  
  else
  {
    return wex::history_frame::AllowClose(id, page);
  }
}

wex::listview* report_sample_frame::GetListView()
{
  return (wex::listview*)m_NotebookWithLists->GetPage(
    m_NotebookWithLists->GetSelection());
}


wex::stc* report_sample_frame::GetSTC()
{
  return m_STC;
}
  
wex::stc* report_sample_frame::OpenFile(
  const wex::path& file, const wex::stc_data& data)
{
  m_STC->GetLexer().Reset();
  m_STC->Open(file, wex::stc_data(data).Flags(wex::stc_data::WIN_DEFAULT));
  
  return m_STC;
}
