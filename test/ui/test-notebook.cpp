////////////////////////////////////////////////////////////////////////////////
// Name:      test-notebook.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/defs.h>
#include <wex/ui/notebook.h>

#include "test.h"

TEST_CASE("wex::notebook")
{
  auto* notebook = new wex::notebook();
  frame()->pane_add(notebook);

  auto* page1 = new wxWindow(frame(), wxID_ANY);
  auto* page2 = new wxWindow(frame(), wxID_ANY);
  auto* page3 = new wxWindow(frame(), wxID_ANY);
  auto* page4 = new wxWindow(frame(), wxID_ANY);
  auto* page5 = new wxWindow(frame(), wxID_ANY);

  REQUIRE(
    notebook->add_page(wex::data::notebook().page(page1).key("key1")) !=
    nullptr);
  REQUIRE(
    notebook->add_page(wex::data::notebook().page(page2).key("key2")) !=
    nullptr);
  REQUIRE(
    notebook->add_page(wex::data::notebook().page(page3).key("key3")) !=
    nullptr);
  // pages: 0,1,2 keys: key1, key2, key3 pages page1,page2,page3.

  SUBCASE("access")
  {
    REQUIRE(notebook->key_by_page(page1) == "key1");
    REQUIRE(notebook->page_by_key("key1") == page1);
    REQUIRE(notebook->page_index_by_key("key1") == 0);
    REQUIRE(notebook->page_index_by_key("xxx") == wxNOT_FOUND);
  }

  SUBCASE("change")
  {
    REQUIRE(notebook->set_page_text("key1", "keyx", "hello"));
    REQUIRE(notebook->page_by_key("keyx") == page1);
    // pages: 0,1,2 keys: keyx, key2, key3 pages page1, page2,page3.
    REQUIRE(notebook->page_index_by_key("key1") == wxNOT_FOUND);

    REQUIRE(notebook->delete_page("keyx"));
    REQUIRE(notebook->page_by_key("keyx") == nullptr);
    REQUIRE(notebook->delete_page("key2"));
    REQUIRE(!notebook->delete_page("xxx"));
    // pages: 0 keys: key3 pages:page3.
  }

  SUBCASE("insert")
  {
    REQUIRE(
      notebook->insert_page(wex::data::notebook().page(page4).key("KEY1")) !=
      nullptr);
    REQUIRE(
      notebook->insert_page(wex::data::notebook().page(page5).key("KEY0")) !=
      nullptr);
    // pages: 0,1,2 keys: KEY0, KEY1, key3 pages: page5,page4,page3.
    REQUIRE(notebook->page_index_by_key("KEY0") == 0);
    REQUIRE(notebook->page_index_by_key("KEY1") == 1);

    REQUIRE(notebook->set_selection("KEY1") == page4);
    REQUIRE(notebook->current_page_key() == "KEY1");
    REQUIRE(notebook->set_selection("key3") == page3);
    REQUIRE(notebook->current_page_key() == "key3");
    REQUIRE(notebook->set_selection("XXX") == nullptr);
    REQUIRE(notebook->current_page_key() == "key3");

    REQUIRE(notebook->change_selection("KEY1") == "key3");
    REQUIRE(notebook->current_page_key() == "KEY1");
    REQUIRE(notebook->change_selection("key3") == "KEY1");
    REQUIRE(notebook->current_page_key() == "key3");
    REQUIRE(notebook->change_selection("XXX") == std::string());
    REQUIRE(notebook->current_page_key() == "key3");

    REQUIRE(notebook->delete_page("KEY0"));
    REQUIRE(notebook->delete_page("KEY1"));
    REQUIRE(notebook->delete_page("key3"));
    REQUIRE(notebook->GetPageCount() == 2); // 5 - 3
  }

  SUBCASE("for_each")
  {
    REQUIRE(notebook->DeleteAllPages());

    auto* stc_x = new ui_stc();
    auto* stc_y = new ui_stc();
    auto* stc_z = new ui_stc();

    REQUIRE(
      notebook->add_page(wex::data::notebook().page(stc_x).key("key1")) !=
      nullptr);
    REQUIRE(
      notebook->add_page(wex::data::notebook().page(stc_y).key("key2")) !=
      nullptr);
    REQUIRE(
      notebook->add_page(wex::data::notebook().page(stc_z).key("key3")) !=
      nullptr);

    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_STC_SET_LEXER));
    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_STC_SET_LEXER_THEME));
    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_STC_SYNC));
    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_CONFIG_GET));
    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_SAVE));
    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_CLOSE_OTHERS));
    REQUIRE(notebook->GetPageCount() == 1);
    REQUIRE(notebook->for_each<ui_stc>(wex::ID_ALL_CLOSE));
    REQUIRE(notebook->GetPageCount() == 0);
  }

  SUBCASE("rearrange")
  {
    notebook->rearrange(wxLEFT);
    notebook->rearrange(wxBOTTOM);
  }

  SUBCASE("split")
  {
    REQUIRE(notebook->DeleteAllPages());

    auto* pagev = new wxWindow(frame(), wxID_ANY);
    REQUIRE(
      notebook->add_page(wex::data::notebook().page(pagev).key("keyv")) !=
      nullptr);
    // split having only one page
    REQUIRE(notebook->split("keyv", wxRIGHT));
    auto* pagew = new wxWindow(frame(), wxID_ANY);
    REQUIRE(
      notebook->add_page(wex::data::notebook().page(pagew).key("keyw")) !=
      nullptr);
    // split using incorrect key
    REQUIRE(!notebook->split("err", wxRIGHT));
    REQUIRE(notebook->split("keyv", wxRIGHT));
    REQUIRE(notebook->GetPageCount() == 2);
  }
}
