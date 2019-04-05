////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/util.h>
#include <wex/report/dir.h>
#include <wex/report/listviewfile.h>
#include <wex/report/stream.h>
#include "test.h"

TEST_CASE("wex::tool_dir")
{
  const wex::tool tool(wex::ID_TOOL_REPORT_FIND);
  
  wex::listview* lv = 
    new wex::listview(wex::listview_data().type(wex::listview_data::FIND));
    
  REQUIRE( tool.id() == wex::ID_TOOL_REPORT_FIND);
  REQUIRE( wex::report::stream::setup_tool(tool, frame(), lv));

  wex::test::add_pane(frame(), lv);
  
  wex::tool_dir dir(
    tool,
    "./",
    "*.cpp;*.h",
    wex::dir::type_t().set());

  REQUIRE( dir.get_statistics().get_elements().get_items().empty());
  REQUIRE( dir.find_files() > 0);
  REQUIRE(!dir.get_statistics().get_elements().get_items().empty());
}

TEST_CASE("wex::report::dir")
{
  wex::report::file* file = new wex::report::file(get_project());
  wex::test::add_pane(frame(), file);
  wex::report::dir* dir = new wex::report::dir(file, wex::test::get_path());  
  REQUIRE(dir->find_files() == 0);
}
