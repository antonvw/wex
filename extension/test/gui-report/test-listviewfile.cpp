////////////////////////////////////////////////////////////////////////////////
// Name:      test-listviewfile.cpp
// Purpose:   Implementation for wxExtension report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/dir.h>
#include <wx/extension/report/listviewfile.h>
#include "test.h"

TEST_CASE("wxExListViewFile")
{
  wxExListViewFile* listView = new wxExListViewFile(GetFrame(), GetFrame(), GetProject());
  
  AddPane(GetFrame(), listView);

  listView->AppendColumn(wxExColumn("String", wxExColumn::COL_STRING));
  listView->AppendColumn(wxExColumn("Number", wxExColumn::COL_INT));

  // Remember that listview file already has columns.
  REQUIRE(listView->FindColumn("String") > 1);
  REQUIRE(listView->FindColumn("Number") > 1);

  REQUIRE(listView->FileLoad(wxExFileName(GetProject())));
  REQUIRE(listView->FileSave(wxExFileName("test-rep.prj.bck")));

  REQUIRE(listView->ItemFromText("test1\ntest2\n"));
  
  REQUIRE(listView->GetContentsChanged());
  listView->ResetContentsChanged();
  REQUIRE(!listView->GetContentsChanged());
  listView->AfterSorting();
  
  listView->AddItems(
    "./",
    "*.h", 
    wxDIR_FILES, 
    false); // join the thread
  
  REQUIRE(remove("test-rep.prj.bck") == 0);
  
  for (auto id : std::vector<int> {
    wxID_ADD, wxID_EDIT, wxID_REPLACE_ALL}) 
  {
    wxPostEvent(listView, wxCommandEvent(wxEVT_MENU, id));
  }
}
