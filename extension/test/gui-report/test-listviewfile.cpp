////////////////////////////////////////////////////////////////////////////////
// Name:      test-listviewfile.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/report/listviewfile.h>
#include "test.h"

#define TEST_PRJ "./test-rep.prj"

void wxExGuiReportTestFixture::testListViewFile()
{
  wxExFrameWithHistory* frame = (wxExFrameWithHistory *)wxTheApp->GetTopWindow();
  wxExListViewFile* listView = new wxExListViewFile(frame, frame, TEST_PRJ);
  
  listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));

  // Remember that listview file already has columns.
  CPPUNIT_ASSERT(listView->FindColumn("String") > 1);
  CPPUNIT_ASSERT(listView->FindColumn("Number") > 1);

  CPPUNIT_ASSERT(listView->FileLoad(wxExFileName(TEST_PRJ)));
  CPPUNIT_ASSERT(listView->FileSave(wxExFileName("test-rep.prj.bck")));

  CPPUNIT_ASSERT(listView->ItemFromText("test1\ntest2\n"));
  
  CPPUNIT_ASSERT(remove("test-rep.prj.bck") == 0);
}
