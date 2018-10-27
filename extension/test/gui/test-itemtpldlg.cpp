////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemtpldlg.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
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
    testitem(const std::string& label, item::type type)
      : item(label, type) {;};
    void SetDialog(item_template_dialog<testitem>* dlg) {;};
  };
};

TEST_CASE("wex::item_template_dialog")
{
  wex::item::UseConfig(false);

  SUBCASE("Basic")
  {
    wex::item_template_dialog<wex::testitem>* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {
        {"fruit", "apple"},
        {"button", wex::item::BUTTON},
        {"string1"},
        {"string2"},
        {"more fruit", "citron"}},
      wex::window_data().Title("3 columns"), 0, 3);
    
    REQUIRE( wex::testitem("test", wex::item::BUTTON).GetType() == wex::item::BUTTON);
    
    REQUIRE(!dlg->BindButton({}));
    REQUIRE(!dlg->BindButton({"test", wex::item::COMBOBOX}));
    REQUIRE(!dlg->BindButton({"test", wex::item::BUTTON})); // not yet laid out0
    REQUIRE(!dlg->BindButton({"test", wex::item::COMBOBOX_DIR})); // same

    REQUIRE( dlg->BindButton(dlg->GetItem("button")));
    
    dlg->Show();
    
    REQUIRE( std::any_cast<std::string>(dlg->GetItem("fruit").GetLabel()) == "fruit");
    REQUIRE( std::any_cast<std::string>(dlg->GetItemValue("fruit")) == "apple");
    REQUIRE(!dlg->GetItemValue("xxx").has_value());
    REQUIRE( std::any_cast<std::string>(dlg->GetItem("xxx").GetLabel()).empty());
    REQUIRE(!dlg->GetItemValue("yyy").has_value());
    
    // asserts in 3.0
#if wxCHECK_VERSION(3,1,0)
    REQUIRE( dlg->SetItemValue("fruit", std::string("strawberry")));
    REQUIRE(!dlg->SetItemValue("xxx", "blueberry"));
    REQUIRE( std::any_cast<std::string>(dlg->GetItemValue("fruit")) == "strawberry");
#endif
    
    dlg->ForceCheckBoxChecked();
  }

  SUBCASE("Test dialog with checkbox item")
  {
    wex::item_template_dialog<wex::testitem>* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {{"checkbox", wex::item::CHECKBOX}},
      wex::window_data().Title("checkbox items"));

    dlg->ForceCheckBoxChecked();
    dlg->Show();
  }

  SUBCASE("Test dialog without buttons")
  {
    wex::item_template_dialog<wex::testitem>* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {
        {"string1"},
        {"string2"}},
      wex::window_data().Button(0).Title("no buttons"));
    dlg->Show();
  }

  SUBCASE("Test dialog without items")
  {
    wex::item_template_dialog<wex::testitem>* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem>(),
      wex::window_data().Title("no items"));
    dlg->Show();
  }
  
  SUBCASE("Test dialog with empty items")
  {
    wex::item_template_dialog<wex::testitem>* dlg = new wex::item_template_dialog<wex::testitem>(
      std::vector <wex::testitem> {{}, {}, {}},
      wex::window_data().Title("empty items"));
    dlg->Show();
  }
}
