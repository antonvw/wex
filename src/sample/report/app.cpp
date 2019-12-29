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
#include <wex/menubar.h>
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

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("wex-sample-rep");

  if (!wex::app::OnInit())
  {
    return false;
  }

  auto* f = new frame();
  f->Show(true);

  return true;
}

frame::frame() 
  : wex::report::frame()
  , m_notebook(new wex::notebook(
      wex::window_data().style(
        wxAUI_NB_DEFAULT_STYLE | wxAUI_NB_WINDOWLIST_BUTTON)))
  , m_stc(new wex::stc())
{
  SetIcon(wxICON(app));

  SetMenuBar(new wex::menubar({
    {new wex::menu({
      {wxID_OPEN}, 
      {}, 
      {wex::menu_item::PRINT}, 
      {}, 
      {wex::menu_item::EXIT}}), 
      "&File"},
    {new wex::menu({this}), "&View"},
    {new wex::menu({
      {wxID_ABOUT, "", "", "", [=](wxCommandEvent& event) {
          wxAboutDialogInfo info;
          info.SetIcon(GetIcon());
          info.SetVersion(wex::get_version_info().get());
          info.SetCopyright(wex::get_version_info().copyright());
          wxAboutBox(info);}}}), "&Help"}}));

  get_toolbar()->add_standard();
  
  setup_statusbar({
    {"PaneFileType", 50},
    {"PaneInfo", 100},
    {"PaneLexer", 60}});

  const wex::lexer lexer = wex::lexers::get()->find("cpp");

  for (int i = wex::listview_data::FOLDER; i <= wex::listview_data::FILE; i++)
  {
    auto* vw = new wex::report::listview(
      wex::listview_data().type((wex::listview_data::type_t)i).lexer(&lexer));

    m_notebook->add_page(
      vw, 
      vw->data().type_description(), 
      vw->data().type_description(), 
      true);
  }

  manager().AddPane(
    m_stc, 
    wxAuiPaneInfo().CenterPane().CloseButton(false).MaximizeButton(true));

  manager().AddPane(
    m_notebook, 
    wxAuiPaneInfo().CloseButton(false).Bottom().MinSize(wxSize(250, 250)));

  manager().AddPane(
    new wex::report::dirctrl(this),
    wxAuiPaneInfo().Caption("DirCtrl").Left().MinSize(wxSize(250, 250)));

  manager().Update();

  wex::report::dir dir(
    (wex::listview*)m_notebook->page_by_key(
      wex::listview_data().type(wex::listview_data::FILE).type_description()),
    wex::path::current(),
    "*.cpp;*.h");

  dir.find_files();

  wex::listitem item(
    (wex::listview*)m_notebook->page_by_key(
      wex::listview_data().type(wex::listview_data::FILE).type_description()),
    wex::path("NOT EXISTING ITEM"));

  item.insert();
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    m_stc->get_file().file_new(wex::path());}, wxID_NEW);
}

wex::report::listview* frame::activate(
  wex::listview_data::type_t type, 
  const wex::lexer* lexer)
{
  for (
    size_t i = 0;
    i < m_notebook->GetPageCount();
    i++)
  {
    wex::report::listview* vw = (wex::report::listview*)
      m_notebook->GetPage(i);

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
              wex::log::verbose(lexer->display_lexer()) << 
                ", only cpp for the sample";
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

bool frame::allow_close(wxWindowID id, wxWindow* page)
{
  if (page == file_history_list())
  {
    // prevent possible crash, if set_recent_file tries
    // to add listitem to deleted history list.
    return false;
  }  
  else
  {
    return wex::report::frame::allow_close(id, page);
  }
}

wex::listview* frame::get_listview()
{
  return (wex::listview*)m_notebook->GetPage(
    m_notebook->GetSelection());
}


wex::stc* frame::get_stc()
{
  return m_stc;
}
  
wex::stc* frame::open_file(
  const wex::path& file, const wex::stc_data& data)
{
  m_stc->get_lexer().clear();
  m_stc->open(file, wex::stc_data(data).flags(0));
  
  return m_stc;
}
