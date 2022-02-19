////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview-file.cpp
// Purpose:   Implementation for wex del unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/del/listview-file.h>

#include "test.h"

TEST_CASE("wex::del::file")
{
  auto* lv = new wex::del::file(get_project());
  del_frame()->pane_add(lv);

  SUBCASE("constructor")
  {
    REQUIRE(lv->get_file().path() == get_project());

    REQUIRE(!lv->text_addfiles().empty());
    REQUIRE(!lv->text_addfolders().empty());
    REQUIRE(!lv->text_addrecursive().empty());
    REQUIRE(!lv->text_addwhat().empty());
    REQUIRE(!lv->text_infolder().empty());
  }

#ifdef __UNIX__
  SUBCASE("add")
  {
    lv->add_items(
      "./",
      "*.h",
      wex::data::dir::type_t().set(wex::data::dir::FILES));
  }
#endif

  SUBCASE("columns")
  {
    lv->append_columns(
      {{"String", wex::column::STRING}, {"Number", wex::column::INT}});

    // Remember that listview file already has columns.
    REQUIRE(lv->find_column("String") > 1);
    REQUIRE(lv->find_column("Number") > 1);
  }

#ifndef __WXMSW__
  SUBCASE("event")
  {
    for (auto id : std::vector<int>{wxID_ADD, wxID_EDIT, wxID_REPLACE_ALL})
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(lv, event);
    }
  }
#endif

  SUBCASE("load-save")
  {
    REQUIRE(lv->file_load(get_project()));
#ifndef __WXMSW__
    // the project file is a unix file
    REQUIRE(lv->GetItemCount() > 10);
#endif
    REQUIRE(lv->file_save(wex::path("test-del.prj.bck")));
    REQUIRE(remove("test-del.prj.bck") == 0);
  }

  SUBCASE("paste")
  {
    lv->clear();
    REQUIRE(lv->item_from_text("test1\ntest2\n"));
    REQUIRE(lv->is_contents_changed());
    REQUIRE(lv->GetItemCount() == 2);
    lv->reset_contents_changed();
    REQUIRE(!lv->is_contents_changed());
    REQUIRE(lv->sort_column("File Name"));
    REQUIRE(!lv->is_contents_changed());
  }
}
