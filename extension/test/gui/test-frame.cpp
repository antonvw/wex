////////////////////////////////////////////////////////////////////////////////
// Name:      test-frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/defs.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::frame")
{
  get_stc()->SetFocus();
  get_stc()->get_file().reset_contents_changed();

  REQUIRE( ((wex::frame *)frame())->open_file(get_testpath("test.h")));
  REQUIRE( ((wex::frame *)frame())->open_file(get_testpath("test.h"), "contents"));
  REQUIRE( ((wex::frame *)frame())->is_open(get_testpath("test.h")));
  REQUIRE(!((wex::frame *)frame())->is_open(wex::path("xxx")));
  
  REQUIRE( frame()->get_grid() == nullptr);
  REQUIRE( frame()->get_listview() == nullptr);
  REQUIRE( frame()->get_stc() != nullptr);
  get_stc()->get_file().reset_contents_changed();
  
  frame()->set_find_focus(frame()->get_stc());
  frame()->set_find_focus(nullptr);
  frame()->set_find_focus(frame());
  
  wxMenuBar* bar = new wxMenuBar();
  wex::menu* menu = new wex::menu();
  menu->append_edit();
  bar->Append(menu, "Edit");
  frame()->SetMenuBar(bar);
  
  frame()->statusbar_clicked("test");
  frame()->statusbar_clicked("Pane1");
  frame()->statusbar_clicked("Pane2");
  
  frame()->statusbar_clicked_right("test");
  frame()->statusbar_clicked_right("Pane1");
  frame()->statusbar_clicked_right("Pane2");
  
  frame()->set_recent_file("testing");
  
  REQUIRE(!frame()->statustext("hello", "test"));
  REQUIRE( frame()->statustext("hello1", "Pane1"));
  REQUIRE( frame()->statustext("hello2", "Pane2"));
  REQUIRE( frame()->get_statustext("Pane1") == "hello1");
  REQUIRE( frame()->get_statustext("Pane2") == "hello2");
  
  REQUIRE(!frame()->update_statusbar(frame()->get_stc(), "test"));
  REQUIRE(!frame()->update_statusbar(frame()->get_stc(), "Pane1"));
  REQUIRE(!frame()->update_statusbar(frame()->get_stc(), "Pane2"));
#ifndef __WXOSX__
  REQUIRE( frame()->update_statusbar(frame()->get_stc(), "PaneInfo"));
#endif
  
  wex::stc* stc = new wex::stc();
  add_pane(frame(), stc);
  stc->SetFocus();
  
  REQUIRE( frame()->get_stc() == stc);
  REQUIRE( frame()->update_statusbar(stc, "PaneInfo"));
  REQUIRE( frame()->update_statusbar(stc, "PaneLexer"));
  REQUIRE( frame()->update_statusbar(stc, "PaneFileType"));
  
  wxCommandEvent event(wxEVT_MENU, wxID_OPEN);
  stc->get_vi().set_register_yank("test.h");
  for (const auto& str : std::vector<wxString> {
    "xxx", "+10 test", "`pwd`", "0"})
  {
    event.SetString(str);
    wxPostEvent(frame(), event);
  }
  
#ifndef __WXMSW__
  for (const auto& id : std::vector<int> {
    wxID_FIND, wxID_REPLACE, 
    wex::ID_VIEW_MENUBAR, wex::ID_VIEW_STATUSBAR, wex::ID_VIEW_TITLEBAR}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(frame(), event);
    wxQueueEvent(frame(), event);
  }
#endif
}
