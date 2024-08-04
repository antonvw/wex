////////////////////////////////////////////////////////////////////////////////
// Name:      test-listview.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log-none.h>
#include <wex/factory/defs.h>
#include <wex/ui/listitem.h>
#include <wex/ui/listview.h>

#include <wx/uiaction.h>

#include "test.h"

TEST_CASE("wex::listview")
{
  auto* lv = new wex::listview();
  frame()->pane_add(lv);

  const std::vector<wex::column> common_cols{
    {"Int", wex::column::INT},
    {"Date", wex::column::DATE},
    {"Float", wex::column::FLOAT},
    {"String", wex::column::STRING}};

  SUBCASE("general")
  {
    REQUIRE(lv->data().type() == wex::data::listview::NONE);

    wex::listview::config_dialog(
      wex::data::window().button(wxAPPLY | wxCANCEL));

    REQUIRE(lv->data().image() == wex::data::listview::IMAGE_ART);

    lv->config_get();

    REQUIRE(lv->field_separator() != 0);

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

    wex::log_none off;
    REQUIRE(!lv->insert_item({"test"}));
    REQUIRE(!lv->insert_item({"1", "2", "3", "4", "5"}));

    REQUIRE(lv->find_next("95"));
    REQUIRE(!lv->find_next("test"));

    REQUIRE(!lv->item_from_text("a new item"));
    REQUIRE(!lv->find_next("another new item"));
    REQUIRE(lv->item_from_text("999"));

    REQUIRE(lv->item_to_text(0).contains("95"));
    REQUIRE(!lv->item_to_text(-1).empty());

    REQUIRE(lv->set_item(0, 3, "new"));
  }

  SUBCASE("clear")
  {
    lv->clear();
    lv->items_update();
    REQUIRE(lv->GetItemCount() == 0);
  }

  SUBCASE("events")
  {
    wex::column intcol(wex::column("Int", wex::column::INT));
    REQUIRE(lv->append_columns({{intcol}}));
    REQUIRE(lv->insert_item({"95"}));

    for (auto id : std::vector<int>{0})
    {
      wxListEvent event(wxEVT_LIST_ITEM_ACTIVATED);
      event.m_itemIndex = id; // for wxWidgets 3.0 compatibility
      wxPostEvent(lv, event);
    }

    for (auto id :
         std::vector<int>{wex::ID_EDIT_SELECT_INVERT, wex::ID_EDIT_SELECT_NONE})
    {
      wxCommandEvent event(wxEVT_MENU, id);
      wxPostEvent(lv, event);
    }
  }

  SUBCASE("item_from_to_text")
  {
    REQUIRE(lv->append_columns({{"String", wex::column::STRING}}));

    REQUIRE(lv->item_from_text("test.h\ntest.h"));
    REQUIRE(!lv->item_to_text(0).empty());
    REQUIRE(!lv->item_to_text(-1).empty());
  }

#ifndef GITHUB
  SUBCASE("popup_menu")
  {
    REQUIRE(lv->append_columns(common_cols));
    REQUIRE(lv->insert_item({"95", "", "", "hello"}));

    lv->SetFocus();
    lv->Select(0);

    wxUIActionSimulator sim;

    // Add some extra distance to take account of window decorations
    wxRect pos;
    lv->GetItemRect(0, pos);
    // We move in slightly so we are not on the edge
    const auto& p(lv->ClientToScreen(pos.GetPosition()) + wxPoint(10, 10));
    REQUIRE(sim.MouseMove(p));
    wxYield();

    REQUIRE(sim.MouseClick(wxMOUSE_BTN_RIGHT));
    REQUIRE(sim.Char(WXK_RETURN));
    wxYield();

    REQUIRE(lv->GetItemCount() == 0);
  }
#endif

  SUBCASE("set_item_image")
  {
    REQUIRE(lv->data().image() == wex::data::listview::IMAGE_ART);
    REQUIRE(!lv->data().type_description().empty());

    REQUIRE(lv->append_columns({{"String", wex::column::STRING}}));

    REQUIRE(lv->item_from_text("test.h\ntest.h"));
    REQUIRE(lv->set_item_image(0, wxART_WARNING));
    lv->items_update();
    REQUIRE(lv->data().image() == wex::data::listview::IMAGE_ART);
    REQUIRE(!lv->data().type_description().empty());

    wex::data::listview data;
    data.image(wex::data::listview::IMAGE_NONE);
    data.type(wex::data::listview::FOLDER);
    auto* ov = new wex::listview(data);
    frame()->pane_add(ov);
    REQUIRE(ov->data().type() == wex::data::listview::FOLDER);
    REQUIRE(ov->data().image() == wex::data::listview::IMAGE_FILE_ICON);
    wex::listitem item(ov, wex::path("./test.h"));
    item.insert();
    REQUIRE(item.GetImage() != -1);
    wex::listitem item2(ov, wex::path("./texxxst.h"));
    item2.insert();
    REQUIRE(item2.GetImage() == -1);
  }

  SUBCASE("sorting")
  {
    REQUIRE(lv->append_columns(common_cols));

    for (int i = 0; i < 10; i++)
    {
      REQUIRE(lv->insert_item(
        {std::to_string(i),
         wex::test::get_path("test.h").stat().get_modification_time_str(),
         std::to_string(static_cast<float>(i) / 2.0),
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

    auto save(lv->save());
    REQUIRE(save.front() == "x\ty\tz");
  }
}
