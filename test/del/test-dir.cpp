////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/dir.h>
#include <wex/del/listview-file.h>
#include <wex/del/stream.h>
#include <wex/frd.h>

#include "test.h"

TEST_CASE("wex::del::dir")
{
  auto* file = new wex::del::file(get_project());
  del_frame()->pane_add(file);
  auto* dir = new wex::del::dir(file, wex::test::get_path());
  REQUIRE(dir->find_files() == 0);
}

TEST_CASE("wex::del::tool_dir")
{
  const wex::tool tool(wex::ID_TOOL_REPORT_FIND);
  REQUIRE(tool.id() == wex::ID_TOOL_REPORT_FIND);

  auto* lv = new wex::del::listview(
    wex::data::listview().type(wex::data::listview::FIND));
  del_frame()->pane_add(lv);

  REQUIRE(wex::del::stream::setup_tool(tool, del_frame(), lv));

  wex::find_replace_data::get()->set_find_string("test");
  wex::del::tool_dir dir(tool, "./", wex::data::dir().file_spec("*.cpp;*.h"));

  REQUIRE(dir.get_statistics().get_elements().get_items().empty());
#ifndef __WXMSW__
  REQUIRE(dir.find_files() > 0);
  REQUIRE(!dir.get_statistics().get_elements().get_items().empty());
#endif
}
