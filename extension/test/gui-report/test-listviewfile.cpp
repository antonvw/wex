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
  wex::listview_file* listView = new wex::listview_file(get_project());
  add_pane(frame(), listView);

  REQUIRE(listView->get_file().get_filename().fullname() == get_project());
  
  REQUIRE(!listView->text_addfiles().empty());
  REQUIRE(!listView->text_addfolders().empty());
  REQUIRE(!listView->text_addrecursive().empty());
  REQUIRE(!listView->text_addwhat().empty());
  REQUIRE(!listView->text_infolder().empty());
  
  listView->append_columns({
    {"String", wex::column::STRING}, 
    {"Number", wex::column::INT}});

  // Remember that listview file already has columns.
  REQUIRE(listView->find_column("String") > 1);
  REQUIRE(listView->find_column("Number") > 1);

  REQUIRE(listView->file_load(get_project()));
  REQUIRE(listView->file_save("test-rep.prj.bck"));
  REQUIRE(remove("test-rep.prj.bck") == 0);

  REQUIRE(listView->item_from_text("test1\ntest2\n"));
  
  REQUIRE( listView->get_contents_changed());
  listView->reset_contents_changed();
  REQUIRE(!listView->get_contents_changed());
  listView->after_sorting();

#ifdef __UNIX__
  listView->add_items(
    "./",
    "*.h", 
    wex::dir::type_t().set(wex::dir::FILES), 
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
