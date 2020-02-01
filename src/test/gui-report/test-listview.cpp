////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/report/listview.h>
#include <wex/report/defs.h>
#include "test.h"

TEST_CASE("wex::report::listview")
{
  SUBCASE("static")
  {
    REQUIRE(wex::report::listview::type_tool(
      wex::tool(wex::ID_TOOL_REPORT_FIND)) == wex::listview_data::FIND);
    REQUIRE(wex::report::listview::type_tool(
      wex::tool(wex::ID_TOOL_REPORT_KEYWORD)) == wex::listview_data::KEYWORD);
  }

  SUBCASE("flow")
  {
    auto* lv = new wex::report::listview(wex::listview_data().type(
      wex::listview_data::FIND));
    wex::test::add_pane(frame(), lv);

    wex::listitem item(lv, wex::test::get_path("test.h"));
    item.insert();
    item.insert();
    item.insert();

    lv->Select(0);
    lv->Select(1);

#ifndef __WXMSW__
    for (auto id : std::vector<int> {
      wex::ID_EDIT_OPEN, 
      wex::ID_EDIT_VCS_LOWEST, 
      wex::report::ID_LIST_COMPARE, 
      wex::report::ID_LIST_RUN_MAKE}) 
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(lv, event);
    }
#endif
  }
  
  SUBCASE("destroy")
  {
    auto* lv = new wex::report::listview(wex::listview_data().type(
      wex::listview_data::FIND));
    lv->Destroy();
  }
}
