////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/listview.h>
#include "test.h"

void fixture::testListViewWithFrame()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  
  CPPUNIT_ASSERT(
    wxExListViewWithFrame::GetTypeTool(tool) == wxExListViewWithFrame::LIST_FIND);
    
  wxExListViewWithFrame* listView = new wxExListViewWithFrame(m_Frame, m_Frame, 
    wxExListViewFileName::LIST_FIND);
  listView->Show();
  
  listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));
  
  CPPUNIT_ASSERT(listView->ItemFromText("test1\ntest2\n"));
  
  listView->InsertItem(1, wxString::Format("item%d", 1));
  listView->SetItem(1, 2, std::to_string(2));
  listView->Select(0);
  listView->Select(1);
  
  for (auto id : std::vector<int> {
    ID_EDIT_OPEN, ID_LIST_COMPARE, ID_LIST_RUN_MAKE,
    ID_EDIT_VCS_LOWEST + 1, ID_TOOL_REPORT_FIND}) 
  {
    wxPostEvent(listView, wxCommandEvent(wxEVT_MENU, id));
  }
  
  listView->Destroy();
}
