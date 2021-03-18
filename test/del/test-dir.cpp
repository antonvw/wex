////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/frd.h>
#include <wex/del/dir.h>
#include <wex/del/listview-file.h>
#include <wex/del/stream.h>
#include <wex/util.h>

TEST_CASE("wex::del::dir")
{
  auto* file = new wex::del::file(get_project());
  add_pane(del_frame(), file);
  auto* dir = new wex::del::dir(file, wex::test::get_path());
  REQUIRE(dir->find_files() == 0);
}

TEST_CASE("wex::del::tool_dir")
{
  const wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  auto* lv =
    new wex::listview(wex::data::listview().type(wex::data::listview::FIND));

  REQUIRE(tool.id() == wex::ID_TOOL_REPORT_FIND);
  REQUIRE(wex::del::stream::setup_tool(tool, del_frame(), lv));

  add_pane(del_frame(), lv);
  wex::find_replace_data::get()->set_find_string("test");
  wex::del::tool_dir dir(
    tool,
    "./",
    wex::data::dir().file_spec("*.cpp;*.h"));

  REQUIRE(dir.get_statistics().get_elements().get_items().empty());
  REQUIRE(dir.find_files() > 0);
  REQUIRE(!dir.get_statistics().get_elements().get_items().empty());
}
