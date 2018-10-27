////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/listitem.h>
#include <wex/report/listview.h>
#include <wex/report/defs.h>
#include "test.h"

TEST_CASE("wex::history_listview")
{
  REQUIRE(wex::history_listview::GetTypeTool(wex::tool(wex::ID_TOOL_REPORT_FIND)) == 
    wex::listview_data::FIND);
  REQUIRE(wex::history_listview::GetTypeTool(wex::tool(wex::ID_TOOL_REPORT_KEYWORD)) == 
    wex::listview_data::KEYWORD);
    
  wex::history_listview* listView = new wex::history_listview(wex::listview_data().Type(wex::listview_data::FIND));
  AddPane(GetFrame(), listView);

  wex::listitem item(listView, GetTestPath("test.h"));
  item.Insert();
  item.Insert();
  item.Insert();

  listView->Select(0);
  listView->Select(1);

#ifndef __WXMSW__
  for (auto id : std::vector<int> {
    wex::ID_EDIT_OPEN, wex::ID_EDIT_VCS_LOWEST, wex::ID_LIST_COMPARE, wex::ID_LIST_RUN_MAKE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView, event);
  }
#endif
  
  wex::history_listview* listView2 = new wex::history_listview(wex::listview_data().Type(wex::listview_data::FIND));
  listView2->Destroy();
}
