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
}
