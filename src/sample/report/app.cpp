////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex report sample classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
  menuFile->append(wxID_OPEN);
  menuFile->append_separator();
  menuFile->append_print();
  menuFile->append_separator();
  menuFile->append(wxID_EXIT);

  wex::menu *menuView = new wex::menu;
  append_panes(menuView);

  wex::menu* menuHelp = new wex::menu;
  menuHelp->append(wxID_ABOUT);

  wxMenuBar *menubar = new wxMenuBar;
  menubar->Append(menuFile, "&File");
  menubar->Append(menuView, "&View");
  menubar->Append(menuHelp, "&Help");
  SetMenuBar(menubar);

  get_toolbar()->add_controls();
  
  setup_statusbar({
    {"PaneFileType", 50},
    {"PaneInfo", 100},
    {"PaneLexer", 60}});

  m_NotebookWithLists = new wex::notebook(
    wex::window_data().style(wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON));

  m_stc = new wex::stc();

  const wex::lexer lexer = wex::lexers::get()->find_by_name("cpp");

  for (int i = wex::listview_data::FOLDER; i <= wex::listview_data::FILE; i++)
  {
    auto* vw = new wex::report::listview(
      wex::listview_data().type((wex::listview_data::type_t)i).lexer(&lexer));

    m_NotebookWithLists->add_page(
      vw, 
      vw->data().type_description(), 
      vw->data().type_description(), 
      true);
  }

  manager().AddPane(
    m_stc, 
    wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));

  manager().AddPane(
    m_NotebookWithLists, 
    wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250)));

  manager().AddPane(
    new wex::dirctrl(this),
    wxAuiPaneInfo().Caption("DirCtrl").Left().MinSize(wxSize(250, 250)));

  manager().Update();

  wex::report::dir dir(
    (wex::listview*)m_NotebookWithLists->page_by_key(
      wex::listview_data().type(wex::listview_data::FILE).type_description()),
    wex::path::current(),
    "*.cpp;*.h");

  dir.find_files();

  wex::listitem item(
    (wex::listview*)m_NotebookWithLists->page_by_key(
      wex::listview_data().type(wex::listview_data::FILE).type_description()),
    wex::path("NOT EXISTING ITEM"));

  item.insert();
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxAboutDialogInfo info;
    info.SetIcon(GetIcon());
    info.SetVersion(wex::get_version_info().get());
    info.SetCopyright(wex::get_version_info().copyright());
    wxAboutBox(info);}, wxID_ABOUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    Close(true);}, wxID_EXIT);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {;}, wxID_HELP);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_stc->get_file().file_new(wex::path());}, wxID_NEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (m_stc->HasCapture())
    {
      m_stc->print_preview();
    }
    else
    {
      auto* lv = get_listview();

      if (lv != nullptr)
      {
        lv->print_preview();
      }
      else
      {
        wex::log::status("No focus");
      }
    }}, wxID_PREVIEW);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    auto* lv = get_listview();

    if (lv != nullptr)
    {
      lv->print();
    }
    else
    {
      wex::log::status("No focus");
    }}, wxID_PRINT);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wex::printing::get()->get_html_printer()->PageSetup();}, wxID_PRINT_SETUP);
}

wex::listview* report_sample_frame::activate(
  wex::listview_data::type_t type, 
  const wex::lexer* lexer)
{
  for (
    size_t i = 0;
    i < m_NotebookWithLists->GetPageCount();
    i++)
  {
    wex::listview* vw = (wex::listview*)m_NotebookWithLists->GetPage(i);

    if (vw->data().type() == type)
    {
      if (type == wex::listview_data::KEYWORD)
      {
        if (lexer != nullptr)
        {
          if (lexer->scintilla_lexer() != "cpp")
          {
            if (!lexer->display_lexer().empty())
            {
              wex::log::verbose(lexer->display_lexer()) << ", only cpp for the sample";
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

bool report_sample_frame::allow_close(wxWindowID id, wxWindow* page)
{
  if (page == file_history_list())
  {
    // prevent possible crash, if set_recent_file tries
    // to add listitem to deleted history list.
    return false;
  }  
  else
  {
    return wex::history_frame::allow_close(id, page);
  }
}

wex::listview* report_sample_frame::get_listview()
{
  return (wex::listview*)m_NotebookWithLists->GetPage(
    m_NotebookWithLists->GetSelection());
}


wex::stc* report_sample_frame::get_stc()
{
  return m_stc;
}
  
wex::stc* report_sample_frame::open_file(
  const wex::path& file, const wex::stc_data& data)
{
  m_stc->get_lexer().reset();
  m_stc->open(file, wex::stc_data(data).flags(0));
  
  return m_stc;
}
