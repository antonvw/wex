////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/frd.h>
#include <wex/report/dir.h>
#include <wex/report/listview-file.h>
#include <wex/report/stream.h>
#include <wex/util.h>

TEST_CASE("wex::report::dir")
{
  auto* file = new wex::report::file(get_project());
  wex::test::add_pane(report_frame(), file);
  auto* dir = new wex::report::dir(file, wex::test::get_path());
  REQUIRE(dir->find_files() == 0);
}

TEST_CASE("wex::report::tool_dir")
{
  const wex::tool tool(wex::ID_TOOL_REPORT_FIND);

  auto* lv =
    new wex::listview(wex::data::listview().type(wex::data::listview::FIND));

  REQUIRE(tool.id() == wex::ID_TOOL_REPORT_FIND);
  REQUIRE(wex::report::stream::setup_tool(tool, report_frame(), lv));

  wex::test::add_pane(report_frame(), lv);
  wex::find_replace_data::get()->set_find_string("test");
  wex::report::tool_dir dir(
    tool,
    "./",
    wex::data::dir().file_spec("*.cpp;*.h"));

  REQUIRE(dir.get_statistics().get_elements().get_items().empty());
  REQUIRE(dir.find_files() > 0);
  REQUIRE(!dir.get_statistics().get_elements().get_items().empty());
}