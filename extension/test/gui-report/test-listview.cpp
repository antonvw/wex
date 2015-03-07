////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/listview.h>
#include "test.h"

void wxExGuiReportTestFixture::testListViewWithFrame()
{
  wxExTool tool(ID_TOOL_REPORT_FIND);
  
  CPPUNIT_ASSERT(
    wxExListViewWithFrame::GetTypeTool(tool) == wxExListViewWithFrame::LIST_FIND);
    
  wxExListViewWithFrame* listView = new wxExListViewWithFrame(m_Frame, m_Frame, 
    wxExListViewFileName::LIST_FIND);
  
  listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));
  
  CPPUNIT_ASSERT(listView->ItemFromText("test1\ntest2\n"));
  
  listView->InsertItem(1, wxString::Format("item%d", 1));
  listView->SetItem(1, 2, wxString::Format("%d", 2));
  listView->Select(0);
  listView->Select(1);
  
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED);
  
  const std::vector<int> ids {
    ID_LIST_OPEN_ITEM, ID_LIST_COMPARE, ID_LIST_RUN_MAKE,
    ID_EDIT_VCS_LOWEST + 1, ID_TOOL_LOWEST + 1}; 
    
  for (auto id : ids)
  {
    event.SetInt(id);
    wxPostEvent(listView, event);
  }
}
