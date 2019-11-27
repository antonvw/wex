////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemtpldlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wex/itemtpldlg.h>
#include <wex/item.h>
#include <wex/report/defs.h>
#include <wex/managedframe.h>
#include "test.h"

namespace wex
{
  class testitem : public item
  {
  public:
    testitem(): item() {;};
    testitem(const std::string& label, const std::string& value = std::string())
      : item(label, value) {;};
    testitem(const std::string& label, item::type_t type)
      : item(label, type) {;};
    void set_dialog(item_template_dialog<testitem>* dlg) {;};
  };
};

TEST_CASE("wex::item_template_dialog")
{
  wex::item::use_config(false);

  SUBCASE("basic")
  {
    wex::item_template_dialog<wex::testitem>* dlg = 
      new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {
        {"fruit", "apple"},
        {"button", wex::item::BUTTON},
        {"string1"},
        {"string2"},
        {"more fruit", "citron"}},
      wex::window_data().title("3 columns"), 0, 3);
    
    REQUIRE( wex::testitem("test", wex::item::BUTTON).type() == wex::item::BUTTON);
    
    REQUIRE(!dlg->bind_button({}));
    REQUIRE(!dlg->bind_button({"test", wex::item::COMBOBOX}));
    REQUIRE(!dlg->bind_button({"test", wex::item::BUTTON})); // not yet laid out0
    REQUIRE(!dlg->bind_button({"test", wex::item::COMBOBOX_DIR})); // same

    REQUIRE( dlg->bind_button(dlg->get_item("button")));
    
    dlg->Show();
    
    REQUIRE( std::any_cast<std::string>(dlg->get_item("fruit").label()) == "fruit");
    REQUIRE( std::any_cast<std::string>(dlg->get_item_value("fruit")) == "apple");
    REQUIRE(!dlg->get_item_value("xxx").has_value());
    REQUIRE( std::any_cast<std::string>(dlg->get_item("xxx").label()).empty());
    REQUIRE(!dlg->get_item_value("yyy").has_value());
    REQUIRE( dlg->set_item_value("fruit", std::string("strawberry")));
    REQUIRE(!dlg->set_item_value("xxx", "blueberry"));
    REQUIRE( std::any_cast<std::string>(dlg->get_item_value("fruit")) == "strawberry");
    
    dlg->force_checkbox_checked();
  }

  SUBCASE("dialog_checkbox")
  {
    wex::item_template_dialog<wex::testitem>* dlg = 
      new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {{"checkbox", wex::item::CHECKBOX}},
      wex::window_data().title("checkbox items"));

    dlg->force_checkbox_checked();
    dlg->Show();
  }

  SUBCASE("dialog_no_buttons")
  {
    wex::item_template_dialog<wex::testitem>* dlg = 
      new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {
        {"string1"},
        {"string2"}},
      wex::window_data().button(0).title("no buttons"));
    dlg->Show();
  }

  SUBCASE("dialog_no_items")
  {
    wex::item_template_dialog<wex::testitem>* dlg = 
      new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem>(),
      wex::window_data().title("no items"));
    dlg->Show();
  }
  
  SUBCASE("dialog_empty_items")
  {
    wex::item_template_dialog<wex::testitem>* dlg = 
      new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {{}, {}, {}},
      wex::window_data().title("empty items"));
    dlg->Show();
  }
}
