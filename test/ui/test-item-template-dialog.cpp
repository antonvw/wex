////////////////////////////////////////////////////////////////////////////////
// Name:      test-item-template-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/defs.h>
#include <wex/test/test.h>
#include <wex/ui/frame.h>
#include <wex/ui/item-template-dialog.h>
#include <wex/ui/item.h>

namespace wex
{
class testitem : public item
{
public:
  static void set_dialog(item_template_dialog<testitem>* dlg) { ; }

  testitem() = default;

  testitem(const std::string& label)
    : item(label, std::string())
  {
    ;
  };

  testitem(const std::string& label, const std::string& value)
    : item(label, value)
  {
    ;
  };

  testitem(const std::string& label, item::type_t type)
    : item(label, type)
  {
    ;
  };
};
}; // namespace wex

TEST_SUITE_BEGIN("wex::item");

TEST_CASE("wex::item_template_dialog")
{
  wex::item::use_config(false);

  SUBCASE("basic")
  {
    auto* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector<wex::testitem>{
        {"fruit", "apple"},
        {"button", wex::item::BUTTON},
        {"string1"},
        {"string2"},
        {"more fruit", "citron"}},
      wex::data::window().title("3 columns"),
      0,
      3);

    REQUIRE(
      wex::testitem("test", wex::item::BUTTON).type() == wex::item::BUTTON);

    REQUIRE(!dlg->bind_button({}));
    REQUIRE(!dlg->bind_button({"test", wex::item::COMBOBOX}));
    REQUIRE(
      !dlg->bind_button({"test", wex::item::BUTTON})); // not yet laid out0
    REQUIRE(!dlg->bind_button({"test", wex::item::COMBOBOX_DIR})); // same

    REQUIRE(dlg->bind_button(dlg->find("button")));

    dlg->Show();

    REQUIRE(std::any_cast<std::string>(dlg->find("fruit").label()) == "fruit");
    REQUIRE(
      std::any_cast<std::string>(dlg->get_item_value("fruit")) == "apple");
    REQUIRE(std::any_cast<std::string>(dlg->get_item_value("xxx")).empty());
    REQUIRE(std::any_cast<std::string>(dlg->find("xxx").label()).empty());
    REQUIRE(dlg->set_item_value("fruit", std::string("strawberry")));
    REQUIRE(!dlg->set_item_value("xxx", "blueberry"));
    REQUIRE(
      std::any_cast<std::string>(dlg->get_item_value("fruit")) == "strawberry");

    dlg->force_checkbox_checked();
  }

  SUBCASE("dialog_checkbox")
  {
    auto* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector<wex::testitem>{{"checkbox", wex::item::CHECKBOX}},
      wex::data::window().title("checkbox items"));

    dlg->force_checkbox_checked();
    dlg->Show();
  }

  SUBCASE("dialog_no_buttons")
  {
    auto* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector<wex::testitem>{{"string1"}, {"string2"}},
      wex::data::window().button(0).title("no buttons"));
    dlg->Show();
  }

  SUBCASE("dialog_no_items")
  {
    auto* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector<wex::testitem>(),
      wex::data::window().title("no items"));
    dlg->Show();
  }

  SUBCASE("dialog_empty_items")
  {
    auto* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector<wex::testitem>{{}, {}, {}},
      wex::data::window().title("empty items"));
    dlg->Show();
  }
}

TEST_SUITE_END();
