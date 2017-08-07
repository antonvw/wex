////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/listitem.h>
#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include "test.h"

TEST_CASE("wxExListViewWithFrame")
{
  REQUIRE(wxExListViewWithFrame::GetTypeTool(wxExTool(ID_TOOL_REPORT_FIND)) == 
    LIST_FIND);
  REQUIRE(wxExListViewWithFrame::GetTypeTool(wxExTool(ID_TOOL_REPORT_KEYWORD)) == 
    LIST_KEYWORD);
    
  wxExListViewWithFrame* listView = new wxExListViewWithFrame(wxExListViewData().Type(LIST_FIND));
  AddPane(GetFrame(), listView);

  wxExListItem item(listView, GetTestFile());
  item.Insert();
  item.Insert();
  item.Insert();

  listView->Select(0);
  listView->Select(1);

#ifndef __WXMSW__
  for (auto id : std::vector<int> {
    ID_EDIT_OPEN, ID_EDIT_VCS_LOWEST, ID_LIST_COMPARE, ID_LIST_RUN_MAKE}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView, event);
  }
#endif
  
  REQUIRE(wxExUIAction(listView));

  wxExListViewWithFrame* listView2 = new wxExListViewWithFrame(wxExListViewData().Type(LIST_FIND));
  listView2->Destroy();
}
