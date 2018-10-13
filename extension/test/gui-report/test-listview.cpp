////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/listitem.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include "test.h"

TEST_CASE("wex::history_listview")
{
  REQUIRE(wex::history_listview::GetTypeTool(wex::tool(wex::ID_TOOL_REPORT_FIND)) == 
    wex::LISTVIEW_FIND);
  REQUIRE(wex::history_listview::GetTypeTool(wex::tool(wex::ID_TOOL_REPORT_KEYWORD)) == 
    wex::LISTVIEW_KEYWORD);
    
  wex::history_listview* listView = new wex::history_listview(wex::listview_data().Type(wex::LISTVIEW_FIND));
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
  
  wex::history_listview* listView2 = new wex::history_listview(wex::listview_data().Type(wex::LISTVIEW_FIND));
  listView2->Destroy();
}
