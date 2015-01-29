////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/notebook.h>
#include <wx/extension/report/util.h>
#include <wx/extension/report/listviewfile.h>
#include "test.h"

#define TEST_PRJ "./test-rep.prj"

void wxExGuiReportTestFixture::testUtil()
{
  wxExNotebook* notebook = new wxExNotebook(m_Frame, m_Frame);
  
  wxExListViewFile* page1 = new wxExListViewFile(m_Frame, m_Frame, TEST_PRJ);
  wxExListViewFile* page2 = new wxExListViewFile(m_Frame, m_Frame, TEST_PRJ);
  
  CPPUNIT_ASSERT( notebook->AddPage(page1, "page1") != NULL);
  CPPUNIT_ASSERT( notebook->AddPage(page2, "page2") != NULL);
  
  CPPUNIT_ASSERT( wxExForEach(notebook, ID_LIST_ALL_ITEMS));
  
  wxExListViewFileName* listView = new wxExListViewFileName(
    m_Frame, wxExListViewFileName::LIST_FILE);
  
  wxExListItem item(listView, wxExFileName("./test.h"));
  item.Insert();

  wxExFileStatistics stat = wxExRun(item, ID_TOOL_REPORT_KEYWORD);
}
