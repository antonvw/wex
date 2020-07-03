////////////////////////////////////////////////////////////////////////////////
// Name:      test-report::file.cpp
// Purpose:   Implementation for wex report unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"
#include <wex/dir.h>
#include <wex/report/listviewfile.h>

TEST_CASE("wex::report::file")
{
  auto* listView = new wex::report::file(get_project());
  wex::test::add_pane(report_frame(), listView);

  REQUIRE(listView->get_file().get_filename().fullname() == get_project());

  REQUIRE(!listView->text_addfiles().empty());
  REQUIRE(!listView->text_addfolders().empty());
  REQUIRE(!listView->text_addrecursive().empty());
  REQUIRE(!listView->text_addwhat().empty());
  REQUIRE(!listView->text_infolder().empty());

  listView->append_columns(
    {{"String", wex::column::STRING}, {"Number", wex::column::INT}});

  // Remember that listview file already has columns.
  REQUIRE(listView->find_column("String") > 1);
  REQUIRE(listView->find_column("Number") > 1);

  REQUIRE(listView->file_load(get_project()));
  REQUIRE(listView->file_save("test-rep.prj.bck"));
  REQUIRE(remove("test-rep.prj.bck") == 0);

  REQUIRE(listView->item_from_text("test1\ntest2\n"));

  REQUIRE(listView->get_contents_changed());
  listView->reset_contents_changed();
  REQUIRE(!listView->get_contents_changed());
  listView->after_sorting();

#ifdef __UNIX__
  listView->add_items("./", "*.h", wex::data::dir::FILES);
#endif

#ifndef __WXMSW__
  for (auto id : std::vector<int>{wxID_ADD, wxID_EDIT, wxID_REPLACE_ALL})
  {
    auto* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(listView, event);
  }
#endif
}
