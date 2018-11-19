////////////////////////////////////////////////////////////////////////////////
// Name:      test-managed_frame.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/managedframe.h>
#include <wex/defs.h>
#include <wex/stc.h>
#include <wex/toolbar.h>
#include <wex/vi.h>
#include "test.h"

// Also test the toolbar (wex::toolbar).
TEST_CASE("wex::managed_frame")
{
  REQUIRE(frame()->allow_close(100, nullptr));
  
  get_stc()->SetFocus();
  get_stc()->Show();
  wex::vi* vi = &get_stc()->get_vi();
  
  wex::ex_command command(":n");
  REQUIRE(!frame()->exec_ex_command(command));
  
  REQUIRE(!frame()->show_ex_command(vi, ""));
  REQUIRE(!frame()->show_ex_command(vi, "x"));
  REQUIRE(!frame()->show_ex_command(vi, "xx"));
  REQUIRE( frame()->show_ex_command(vi, "/"));
  REQUIRE( frame()->show_ex_command(vi, "?"));
  REQUIRE( frame()->show_ex_command(vi, "="));
  
  REQUIRE(!frame()->save_current_page("key"));
  REQUIRE( frame()->restore_page("key") == nullptr);
  
  frame()->hide_ex_bar(wex::managed_frame::HIDE_BAR);
  frame()->hide_ex_bar(wex::managed_frame::HIDE_BAR_FOCUS_STC);
  frame()->hide_ex_bar(wex::managed_frame::HIDE_BAR_FORCE);
  frame()->hide_ex_bar(wex::managed_frame::HIDE_BAR_FORCE_FOCUS_STC);
  
  REQUIRE(!frame()->manager().GetPane("VIBAR").IsShown());
  
  frame()->file_history().clear();
  
  wxMenu* menu = new wxMenu();
  frame()->file_history().use_menu(1000, menu);
  frame()->set_find_focus(frame()->get_stc());
  frame()->open_file(GetTestPath("test.h"));
  
  frame()->set_recent_file(GetTestPath("test.h"));
  frame()->set_recent_file("testing");
  
  REQUIRE( frame()->file_history().get_history_file().data().string().find("test.h") != std::string::npos);
  REQUIRE( frame()->file_history().size() > 0);
  REQUIRE(!frame()->file_history().get_history_files(5).empty());
  
  frame()->show_ex_message("hello from frame()");
  REQUIRE(!frame()->show_pane("xxxx"));
  REQUIRE(!frame()->show_pane("xxxx", false));
  frame()->print_ex(vi, "hello vi");
  
  frame()->sync_all();
  frame()->sync_close_all(0);
  
  REQUIRE( frame()->get_toolbar() != nullptr);
  REQUIRE( frame()->get_options_toolbar() != nullptr);
  
  frame()->get_toolbar()->add_controls();
  REQUIRE( frame()->toggle_pane("FINDBAR"));
  REQUIRE( frame()->manager().GetPane("FINDBAR").IsShown());
  REQUIRE( frame()->toggle_pane("OPTIONSBAR"));
  REQUIRE( frame()->manager().GetPane("OPTIONSBAR").IsShown());
  REQUIRE( frame()->toggle_pane("TOOLBAR"));
  REQUIRE(!frame()->manager().GetPane("TOOLBAR").IsShown());
  REQUIRE( frame()->show_pane("TOOLBAR"));
  REQUIRE( frame()->toggle_pane("VIBAR"));
  REQUIRE( frame()->manager().GetPane("VIBAR").IsShown());
  
  REQUIRE(!frame()->toggle_pane("XXXXBAR"));
  REQUIRE(!frame()->manager().GetPane("XXXXBAR").IsOk());
  
  frame()->on_notebook(100, get_stc());
  
  frame()->append_panes(menu);

#ifndef __WXMSW__
  for (auto id : std::vector<int> {
    wxID_PREFERENCES, 
    wex::ID_FIND_FIRST, wex::ID_FIND_LAST,
    wex::ID_CLEAR_FILES, wex::ID_CLEAR_FINDS,
    wex::ID_VIEW_LOWEST + 1, wex::ID_VIEW_LOWEST + 2, wex::ID_VIEW_LOWEST + 3, wex::ID_VIEW_LOWEST + 4}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(frame(), event);
  }
#endif
}
