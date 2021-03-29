////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview-file.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/listview-file.h>

#include "test.h"

TEST_CASE("wex::del::file")
{
  auto* lv = new wex::del::file(get_project());
  del_frame()->pane_add(lv);

  REQUIRE(lv->get_file().get_filename().fullname() == get_project());

  REQUIRE(!lv->text_addfiles().empty());
  REQUIRE(!lv->text_addfolders().empty());
  REQUIRE(!lv->text_addrecursive().empty());
  REQUIRE(!lv->text_addwhat().empty());
  REQUIRE(!lv->text_infolder().empty());

  lv->append_columns(
    {{"String", wex::column::STRING}, {"Number", wex::column::INT}});

  // Remember that listview file already has columns.
  REQUIRE(lv->find_column("String") > 1);
  REQUIRE(lv->find_column("Number") > 1);

  REQUIRE(lv->file_load(get_project()));
  REQUIRE(lv->file_save("test-rep.prj.bck"));
  REQUIRE(remove("test-rep.prj.bck") == 0);

  REQUIRE(lv->item_from_text("test1\ntest2\n"));

  REQUIRE(lv->is_contents_changed());
  lv->reset_contents_changed();
  REQUIRE(!lv->is_contents_changed());
  REQUIRE(lv->sort_column("File Name"));
  REQUIRE(!lv->is_contents_changed());

#ifdef __UNIX__
  lv->add_items("./", "*.h", wex::data::dir::FILES);
#endif

#ifndef __WXMSW__
  for (auto id : std::vector<int>{wxID_ADD, wxID_EDIT, wxID_REPLACE_ALL})
  {
    auto* event = new wxCommandEvent(wxEVT_MENU, id);
    wxQueueEvent(lv, event);
  }
#endif
}
