////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/defs.h>
#include <wex/listview.h>

#include "test.h"

TEST_CASE("wex::listview")
{
  auto* lv = new wex::listview();
  frame()->pane_add(lv);

  SUBCASE("general")
  {
    REQUIRE(lv->data().type() == wex::data::listview::NONE);

    wex::listview::config_dialog(
      wex::data::window().button(wxAPPLY | wxCANCEL));

    REQUIRE(lv->data().image() == wex::data::listview::IMAGE_ART);

    lv->config_get();

    wex::column intcol(wex::column("Int", wex::column::INT));
    REQUIRE(!intcol.is_sorted_ascending());
    intcol.set_is_sorted_ascending(wex::SORT_ASCENDING);
    REQUIRE(intcol.is_sorted_ascending());

    REQUIRE(lv->append_columns({{intcol}}));
    REQUIRE(lv->append_columns(
      {{"Date", wex::column::DATE},
       {"Float", wex::column::FLOAT},
       {"String", wex::column::STRING}}));

    REQUIRE(lv->find_column("Int") == 0);
    REQUIRE(lv->find_column("Date") == 1);
    REQUIRE(lv->find_column("Float") == 2);
    REQUIRE(lv->find_column("String") == 3);

    REQUIRE(lv->insert_item({"95"}));
    REQUIRE(lv->insert_item({"95", "", "", "100"}));
    REQUIRE(!lv->insert_item({"test"}));
    REQUIRE(!lv->insert_item({"1", "2", "3", "4", "5"}));

    REQUIRE(lv->find_next("95"));
    REQUIRE(!lv->find_next("test"));

    REQUIRE(!lv->item_from_text("a new item"));
    REQUIRE(!lv->find_next("another new item"));
    REQUIRE(lv->item_from_text("999"));

    REQUIRE(lv->item_to_text(0).find("95") != std::string::npos);
    REQUIRE(!lv->item_to_text(-1).empty());
  }

  SUBCASE("clear")
  {
    lv->clear();
    lv->items_update();
    REQUIRE(lv->GetItemCount() == 0);
  }

  SUBCASE("sorting")
  {
    REQUIRE(lv->append_columns(
      {{"Int", wex::column::INT},
       {"Date", wex::column::DATE},
       {"Float", wex::column::FLOAT},
       {"String", wex::column::STRING}}));

    for (int i = 0; i < 10; i++)
    {
      REQUIRE(lv->insert_item(
        {std::to_string(i),
         wex::test::get_path("test.h").stat().get_modification_time(),
         std::to_string((float)i / 2.0),
         "hello " + std::to_string(i)}));
    }

    REQUIRE(!lv->sort_column("xxx"));
    REQUIRE(lv->sort_column("Int", wex::SORT_ASCENDING));
    REQUIRE(lv->get_item_text(0, "Int") == "0");
    REQUIRE(lv->get_item_text(1, "Int") == "1");
    REQUIRE(lv->sort_column("Int", wex::SORT_DESCENDING));
    REQUIRE(lv->get_item_text(0, "Int") == "9");
    REQUIRE(lv->get_item_text(1, "Int") == "8");
    REQUIRE(lv->sort_column("Date"));
    REQUIRE(lv->sort_column("Float"));
    REQUIRE(lv->sort_column("String"));

    REQUIRE(lv->sorted_column_no() == 3);
    lv->sort_column_reset();
    REQUIRE(lv->sorted_column_no() == -1);

    lv->SetItem(0, 1, "incorrect date");
    REQUIRE(lv->sort_column("Date"));
  }

  SUBCASE("item_from_to_text")
  {
    REQUIRE(lv->append_columns({{"String", wex::column::STRING}}));

    REQUIRE(lv->item_from_text("test.h\ntest.h"));
    REQUIRE(lv->set_item_image(0, wxART_WARNING));
    lv->items_update();
    REQUIRE(lv->data().image() == wex::data::listview::IMAGE_ART);
    REQUIRE(!lv->data().type_description().empty());
    REQUIRE(!lv->item_to_text(0).empty());
    REQUIRE(!lv->item_to_text(-1).empty());
  }

  SUBCASE("events")
  {
    for (auto id : std::vector<int>{0})
    {
      auto* event        = new wxListEvent(wxEVT_LIST_ITEM_ACTIVATED);
      event->m_itemIndex = id; // for wxWidgets 3.0 compatibility
      wxQueueEvent(lv, event);
    }

    for (auto id :
         std::vector<int>{wex::ID_EDIT_SELECT_INVERT, wex::ID_EDIT_SELECT_NONE})
    {
      auto* event = new wxCommandEvent(wxEVT_MENU, id);
      wxQueueEvent(lv, event);
    }
  }

  SUBCASE("TSV")
  {
    auto* lv =
      new wex::listview(wex::data::listview().type(wex::data::listview::TSV));
    frame()->pane_add(lv);

    REQUIRE(lv->GetColumnCount() == 0);
    REQUIRE(lv->data().type() == wex::data::listview::TSV);
    REQUIRE(lv->load({"x\ty\tz", "v\tw\tx", "a\tb\tc", "f\tg\th"}));
    REQUIRE(lv->GetColumnCount() == 3);
    REQUIRE(lv->GetItemCount() == 4);
  }
}
