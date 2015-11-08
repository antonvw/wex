////////////////////////////////////////////////////////////////////////////////
// Name:      test-listviewfile.cpp
// Purpose:   Implementation for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>
#include <wx/extension/report/listviewfile.h>
#include "test.h"

void fixture::testListViewFile()
{
  wxExListViewFile* listView = new wxExListViewFile(m_Frame, m_Frame, m_Project);
  
  AddPane(m_Frame, listView);

  listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));

  // Remember that listview file already has columns.
  CPPUNIT_ASSERT(listView->FindColumn("String") > 1);
  CPPUNIT_ASSERT(listView->FindColumn("Number") > 1);

  CPPUNIT_ASSERT(listView->FileLoad(wxExFileName(m_Project)));
  CPPUNIT_ASSERT(listView->FileSave(wxExFileName("test-rep.prj.bck")));

  CPPUNIT_ASSERT(listView->ItemFromText("test1\ntest2\n"));
  
  CPPUNIT_ASSERT(listView->GetContentsChanged());
  listView->ResetContentsChanged();
  CPPUNIT_ASSERT(!listView->GetContentsChanged());
  listView->AfterSorting();
  
  listView->AddItems(
    "./",
    "*.h", 
    wxDIR_FILES, 
    false); // join the thread
  
  CPPUNIT_ASSERT(remove("test-rep.prj.bck") == 0);
  
  for (auto id : std::vector<int> {
    wxID_ADD, wxID_EDIT, wxID_REPLACE_ALL}) 
  {
    wxPostEvent(listView, wxCommandEvent(wxEVT_MENU, id));
  }
}
