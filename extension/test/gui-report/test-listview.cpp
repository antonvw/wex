////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/listview.h>
#include <wx/extension/report/defs.h>
#include "test.h"

TEST_CASE("wxExListViewWithFrame")
{
  REQUIRE(wxExListViewWithFrame::GetTypeTool(wxExTool(ID_TOOL_REPORT_FIND)) == 
    wxExListViewWithFrame::LIST_FIND);
  REQUIRE(wxExListViewWithFrame::GetTypeTool(wxExTool(ID_TOOL_REPORT_REPLACE)) == 
    wxExListViewWithFrame::LIST_REPLACE);
  REQUIRE(wxExListViewWithFrame::GetTypeTool(wxExTool(ID_TOOL_REPORT_KEYWORD)) == 
    wxExListViewWithFrame::LIST_KEYWORD);
    
  wxExListViewWithFrame* listView = new wxExListViewWithFrame(GetFrame(), GetFrame(), 
    wxExListView::LIST_FIND);
  
  AddPane(GetFrame(), listView);

  listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));
  
  REQUIRE(listView->ItemFromText("test1\ntest2\n"));
  
  listView->InsertItem(1, wxString::Format("item%d", 1));
  listView->SetItem(1, 2, std::to_string(2));
  listView->Select(0);
  listView->Select(1);
  
  for (auto id : std::vector<int> {
    ID_EDIT_OPEN, ID_LIST_COMPARE, ID_LIST_RUN_MAKE,
    ID_EDIT_VCS_LOWEST, ID_TOOL_REPORT_FIND}) 
  {
    wxPostEvent(listView, wxCommandEvent(wxEVT_MENU, id));
  }
  
  TestAndContinue(listView, [](wxWindow* window) {
    wxPostEvent(window, wxMouseEvent(wxEVT_RIGHT_DOWN));});

  wxExListViewWithFrame* listView2 = new wxExListViewWithFrame(GetFrame(), GetFrame(), 
    wxExListView::LIST_FIND);
  listView2->Destroy();
}
