////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/util.h>
#include <wex/report/dir.h>
#include <wex/report/listviewfile.h>
#include <wex/report/stream.h>
#include "test.h"

TEST_CASE("wex::tool_dir")
{
  const wex::tool tool = wex::ID_TOOL_REPORT_FIND;

  wex::listview* report = new wex::listview(wex::listview_data().type(wex::listview_data::FIND));
    
  if (!wex::listview_stream::setup_tool(tool, frame(), report))
  {
    return;
  }

  add_pane(frame(), report);
  
  wex::tool_dir dir(
    tool,
    "./",
    "*.cpp;*.h",
    wex::dir::type_t().set());

  dir.find_files();

  wex::log_status(tool.info(&dir.get_statistics().get_elements()));
}

TEST_CASE("wex::listview_dir")
{
  wex::listview_file* listView = new wex::listview_file(get_project());
  add_pane(frame(), listView);
  wex::listview_dir* dir = new wex::listview_dir(listView, get_testpath());
  REQUIRE(dir->find_files() == 0);
}
