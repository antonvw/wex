////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-no-config.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/item.h>
#include <wx/numformatter.h>

#include "test-item.h"
#include "test.h"

TEST_CASE("wex::item-no-config")
{
  ITEM_START()

  SUBCASE("no-config")
  {
    wex::item::use_config(false);

    wex::item item(
      "item",
      "hello string",
      wex::item::TEXTCTRL,
      wex::data::item(wex::data::control().is_required(true)));

    REQUIRE(item.data().columns() == 1);
    REQUIRE(
      std::any_cast<std::string>(item.data().initial()) == "hello string");
    REQUIRE(item.data().control().is_required());
    REQUIRE(item.label() == "item");
    REQUIRE(item.page().empty());
    REQUIRE(item.type() == wex::item::TEXTCTRL);
    REQUIRE(item.window() == nullptr);
    REQUIRE(item.get_value().has_value());
    REQUIRE(!item.is_row_growable());
    REQUIRE(!item.data().apply());

    REQUIRE(!item.to_config(false));
    wex::item::use_config(true);
    REQUIRE(!item.to_config(false));
    wex::item::use_config(false);

    item.set_dialog(nullptr);

    // setting value if window is nullptr should have no effect.
    REQUIRE(!item.set_value("test"));
    REQUIRE(item.get_value().has_value());

    REQUIRE(item.layout(panel, sizer) != nullptr);
    REQUIRE(item.window() != nullptr);
    REQUIRE(std::any_cast<std::string>(item.get_value()) == "hello string");
    REQUIRE(item.set_value(std::string("value changed")));
    REQUIRE(std::any_cast<std::string>(item.get_value()) == "value changed");
    REQUIRE(
      std::any_cast<std::string>(item.data().initial()) == "hello string");

    item.set_row_growable(true);
    REQUIRE(item.is_row_growable());

    wex::item item_int("int", wex::item::TEXTCTRL_INT, std::string("100"));

    REQUIRE(item_int.type() == wex::item::TEXTCTRL_INT);
    REQUIRE(item_int.layout(panel, sizer) != nullptr);
    REQUIRE(item_int.window() != nullptr);
    REQUIRE(std::any_cast<long>(item_int.get_value()) == 100);
    REQUIRE(std::any_cast<std::string>(item_int.data().initial()) == "100");
    REQUIRE(item_int.set_value(300l));
    REQUIRE(std::any_cast<long>(item_int.get_value()) == 300);
    REQUIRE(std::any_cast<std::string>(item_int.data().initial()) == "100");

    wex::item item_int2("int", wex::item::TEXTCTRL_INT, std::string("xxx"));
    REQUIRE(item_int2.type() == wex::item::TEXTCTRL_INT);
    REQUIRE(item_int2.layout(panel, sizer) != nullptr);
    REQUIRE(item_int2.window() != nullptr);
    REQUIRE(std::any_cast<long>(item_int2.get_value()) == 0);

    wex::item item_float(
      "float",
      wex::item::TEXTCTRL_FLOAT,
      std::string("100") +
        std::string(1, wxNumberFormatter::GetDecimalSeparator()) +
        std::string("001"));

    REQUIRE(item_float.type() == wex::item::TEXTCTRL_FLOAT);
    item_float.layout(panel, sizer);
    // wxTextCtrl does not yet respect the locale?
    REQUIRE(std::any_cast<double>(item_float.get_value()) <= 100.001);

    wex::item
      item_spin("spindouble", 20.0, 30.0, 25.0, wex::data::item().inc(0.1));
    REQUIRE(item_spin.type() == wex::item::SPINCTRLDOUBLE);

#ifdef __UNIX__
    wex::item item_picker(
      "picker",
      wex::item::FILEPICKERCTRL,
      std::string("/usr/bin/git"));
#endif

#ifdef __UNIX__
    REQUIRE(item_picker.layout(panel, sizer) != nullptr);
    REQUIRE(
      std::any_cast<std::string>(item_picker.get_value()) == "/usr/bin/git");
#endif

    std::vector<wex::item> items{
      item,
      item_int,
      item_spin
#ifdef __UNIX__
      ,
      item_picker
#endif
    };

    const auto more(wex::test_item().vector());
    items.insert(items.end(), more.begin(), more.end());

    // layout the items and check control is created.
    for (auto& it : items)
    {
      // wex::item::USER is not yet laid out ok, gives errors.
      if (it.type() != wex::item::USER)
      {
        // Testing on not nullptr not possible,
        // not all items need a sizer.
        it.layout(panel, sizer);
      }

      if (it.type() != wex::item::EMPTY && it.type() != wex::item::SPACER)
      {
        CAPTURE(it.type());
        REQUIRE(it.window() != nullptr);
      }
    }
  }
}
