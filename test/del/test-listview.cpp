////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/defs.h>
#include <wex/del/listview.h>
#include <wex/listitem.h>

#include "test.h"

TEST_CASE("wex::del::listview")
{
  SUBCASE("static")
  {
    REQUIRE(
      wex::del::listview::type_tool(wex::tool(wex::ID_TOOL_REPORT_FIND)) ==
      wex::data::listview::FIND);
  }

  SUBCASE("flow")
  {
    auto* lv = new wex::del::listview(
      wex::data::listview().type(wex::data::listview::FIND));
    del_frame()->pane_add(lv);

    wex::listitem item(lv, wex::test::get_path("test.h"));
    item.insert();
    item.insert();
    item.insert();

    lv->Select(0);
    lv->Select(1);

#ifndef __WXMSW__
    for (auto id : std::vector<int>{
           wex::ID_EDIT_OPEN,
           wex::ID_EDIT_VCS_LOWEST,
           wex::del::ID_LIST_COMPARE,
           wex::del::ID_LIST_RUN_MAKE})
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(lv, event);
    }
#endif
  }

#ifndef __WXMSW__
  SUBCASE("destroy")
  {
    auto* lv = new wex::del::listview(
      wex::data::listview().type(wex::data::listview::FIND));
    lv->Destroy();
  }
#endif
}
