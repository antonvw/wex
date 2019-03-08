////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/report/listview.h>
#include <wex/report/defs.h>
#include "test.h"

TEST_CASE("wex::report::listview")
{
  REQUIRE(wex::report::listview::type_tool(wex::tool(wex::ID_TOOL_REPORT_FIND)) == 
    wex::listview_data::FIND);
  REQUIRE(wex::report::listview::type_tool(wex::tool(wex::ID_TOOL_REPORT_KEYWORD)) == 
    wex::listview_data::KEYWORD);
    
  wex::report::listview* listView = 
    new wex::report::listview(wex::listview_data().type(wex::listview_data::FIND));
  wex::test::add_pane(frame(), listView);

  wex::listitem item(listView, wex::test::get_path("test.h"));
  item.insert();
  item.insert();
  item.insert();

  listView->Select(0);
  listView->Select(1);

#ifndef __WXMSW__
  for (auto id : std::vector<int> {
    wex::ID_EDIT_OPEN, 
    wex::ID_EDIT_VCS_LOWEST, 
    wex::ID_LIST_COMPARE, 
    wex::ID_LIST_RUN_MAKE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView, event);
  }
#endif
  
  wex::report::listview* listView2 = 
    new wex::report::listview(wex::listview_data().type(wex::listview_data::FIND));
  listView2->Destroy();
}
