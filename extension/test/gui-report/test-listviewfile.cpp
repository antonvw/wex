////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview_file.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/dir.h>
#include <wex/report/listviewfile.h>
#include "test.h"

TEST_CASE("wex::listview_file")
{
  wex::listview_file* listView = new wex::listview_file(GetProject());
  AddPane(GetFrame(), listView);

  REQUIRE(listView->GetFile().GetFileName().GetFullName() == GetProject());
  
  REQUIRE(!listView->GetTextAddFiles().empty());
  REQUIRE(!listView->GetTextAddFolders().empty());
  REQUIRE(!listView->GetTextAddRecursive().empty());
  REQUIRE(!listView->GetTextAddWhat().empty());
  REQUIRE(!listView->GetTextInFolder().empty());
  
  listView->AppendColumns({
    {"String", wex::column::STRING}, 
    {"Number", wex::column::INT}});

  // Remember that listview file already has columns.
  REQUIRE(listView->FindColumn("String") > 1);
  REQUIRE(listView->FindColumn("Number") > 1);

  REQUIRE(listView->FileLoad(GetProject()));
  REQUIRE(listView->FileSave("test-rep.prj.bck"));
  REQUIRE(remove("test-rep.prj.bck") == 0);

  REQUIRE(listView->ItemFromText("test1\ntest2\n"));
  
  REQUIRE( listView->GetContentsChanged());
  listView->ResetContentsChanged();
  REQUIRE(!listView->GetContentsChanged());
  listView->AfterSorting();

#ifdef __UNIX__
  listView->AddItems(
    "./",
    "*.h", 
    wex::dir::FILES, 
    false); // join the thread
#endif

#ifndef __WXMSW__
  for (auto id : std::vector<int> {
    wxID_ADD, wxID_EDIT, wxID_REPLACE_ALL}) 
  {
    wxCommandEvent* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView, event);
  }
#endif
}
