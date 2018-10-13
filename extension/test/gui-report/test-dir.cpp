////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/util.h>
#include <wx/extension/report/dir.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/stream.h>
#include "test.h"

TEST_CASE("wex::tool_dir")
{
  const wex::tool tool = wex::ID_TOOL_REPORT_FIND;

  wex::listview* report = new wex::listview(wex::listview_data().Type(wex::LISTVIEW_FIND));
    
  if (!wex::listview_stream::SetupTool(tool, GetFrame(), report))
  {
    return;
  }

  AddPane(GetFrame(), report);
  
  wex::tool_dir dir(
    tool,
    "./",
    "*.cpp;*.h",
    wex::DIR_FILES | wex::DIR_HIDDEN | wex::DIR_DIRS);

  dir.FindFiles();

  wex::log_status(tool.Info(&dir.GetStatistics().GetElements()));
}

TEST_CASE("wex::listview_dir")
{
  wex::listview_file* listView = new wex::listview_file(GetProject());
  AddPane(GetFrame(), listView);
  wex::listview_dir* dir = new wex::listview_dir(listView, GetTestPath());
  REQUIRE(dir->FindFiles() == 0);
}
