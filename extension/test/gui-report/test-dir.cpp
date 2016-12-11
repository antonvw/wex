////////////////////////////////////////////////////////////////////////////////
// Name:      test-dir.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/dir.h>
#include <wx/extension/report/listviewfile.h>
#include <wx/extension/report/stream.h>
#include "test.h"

TEST_CASE("wxExDirTool")
{
  const wxExTool tool = ID_TOOL_REPORT_FIND;

  wxExListView* report = new wxExListView(
    GetFrame(), 
    wxExListView::LIST_FIND);
    
  if (!wxExStreamToListView::SetupTool(tool, GetFrame(), report))
  {
    return;
  }

  AddPane(GetFrame(), report);
  
  wxExDirTool dir(
    tool,
    "./",
    "*.cpp;*.h",
    wxDIR_FILES | wxDIR_HIDDEN | wxDIR_DIRS);

  dir.FindFiles();

  wxLogStatus(tool.Info(&dir.GetStatistics().GetElements()));
}

TEST_CASE("wxExDirWithListView")
{
  wxExListViewFile* listView = new wxExListViewFile(GetFrame(), GetFrame(), GetProject());
  AddPane(GetFrame(), listView);
  wxExDirWithListView* dir = new wxExDirWithListView(listView, GetTestDir());
  REQUIRE(dir->FindFiles() > 0);
}
