////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview-file.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/listview-file.h>
#include <wex/dir.h>

#include "test.h"

TEST_CASE("wex::del::file")
{
  auto* listView = new wex::del::file(get_project());
  del_frame()->pane_add(listView);

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

  REQUIRE(listView->is_contents_changed());
  listView->reset_contents_changed();
  REQUIRE(!listView->is_contents_changed());
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
