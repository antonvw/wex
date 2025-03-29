////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/defs.h>
#include <wex/del/listview.h>
#include <wex/ui/listitem.h>

#include <wx/uiaction.h>

#include "test.h"

TEST_CASE("wex::del::listview")
{
  SECTION("flow")
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

    wex::config(_("list.Comparator")).set("diff");

#ifndef __WXMSW__
    // do not test VCS, it results in modal dialog
    for (auto id : std::vector<int>{
           wex::ID_EDIT_OPEN,
           wex::del::ID_LIST_COMPARE,
           wex::del::ID_LIST_RUN_BUILD})
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(lv, event);
    }
#endif
  }

  SECTION("static")
  {
    REQUIRE(
      wex::del::listview::type_tool(wex::tool(wex::ID_TOOL_REPORT_FIND)) ==
      wex::data::listview::FIND);
    REQUIRE(
      wex::del::listview::type_tool(wex::tool(wex::ID_SHELL_APPEND)) ==
      wex::data::listview::NONE);
  }

#ifndef __WXMSW__
  SECTION("destroy")
  {
    auto* lv = new wex::del::listview(
      wex::data::listview().type(wex::data::listview::FIND));
    lv->Destroy();
  }
#endif
}
