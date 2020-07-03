////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/listitem.h>
#include <wex/report/defs.h>
#include <wex/report/listview.h>

TEST_CASE("wex::report::listview")
{
  SUBCASE("static")
  {
    REQUIRE(
      wex::report::listview::type_tool(wex::tool(wex::ID_TOOL_REPORT_FIND)) ==
      wex::data::listview::FIND);
    REQUIRE(
      wex::report::listview::type_tool(
        wex::tool(wex::ID_TOOL_REPORT_KEYWORD)) == wex::data::listview::KEYWORD);
  }

  SUBCASE("flow")
  {
    auto* lv = new wex::report::listview(
      wex::data::listview().type(wex::data::listview::FIND));
    wex::test::add_pane(report_frame(), lv);

    wex::listitem item(lv, wex::test::get_path("test.h"));
    item.insert();
    item.insert();
    item.insert();

    lv->Select(0);
    lv->Select(1);

#ifndef __WXMSW__
    for (auto id : std::vector<int>{wex::ID_EDIT_OPEN,
                                    wex::ID_EDIT_VCS_LOWEST,
                                    wex::report::ID_LIST_COMPARE,
                                    wex::report::ID_LIST_RUN_MAKE})
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(lv, event);
    }
#endif
  }

#ifndef __WXMSW__
  SUBCASE("destroy")
  {
    auto* lv = new wex::report::listview(
      wex::data::listview().type(wex::data::listview::FIND));
    lv->Destroy();
  }
#endif
}
