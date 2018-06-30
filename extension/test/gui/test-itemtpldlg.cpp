////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemtpldlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/itemtpldlg.h>
#include <wx/extension/item.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/managedframe.h>
#include "test.h"

class wxExTestItem : public wxExItem
{
public:
  wxExTestItem(): wxExItem() {;};
  wxExTestItem(const std::string& label, const std::string& value = std::string())
    : wxExItem(label, value) {;};
  wxExTestItem(const std::string& label, wxExItemType type)
    : wxExItem(label, type) {;};
  void SetDialog(wxExItemTemplateDialog<wxExTestItem>* dlg) {;};
};

TEST_CASE("wxExItemTemplateDialog")
{
  wxExItem::UseConfig(false);

  SUBCASE("Basic")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(
      std::vector <wxExTestItem> {
        {"fruit", "apple"},
        {"button", ITEM_BUTTON},
        {"string1"},
        {"string2"},
        {"more fruit", "citron"}},
      wxExWindowData().Title("3 columns"), 0, 3);
    
    REQUIRE( wxExTestItem("test", ITEM_BUTTON).GetType() == ITEM_BUTTON);
    
    REQUIRE(!dlg->BindButton({}));
    REQUIRE(!dlg->BindButton({"test", ITEM_COMBOBOX}));
    REQUIRE(!dlg->BindButton({"test", ITEM_BUTTON})); // not yet laid out0
    REQUIRE(!dlg->BindButton({"test", ITEM_COMBOBOX_DIR})); // same

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
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(
      std::vector <wxExTestItem> {{"checkbox", ITEM_CHECKBOX}},
      wxExWindowData().Title("checkbox items"));

    dlg->ForceCheckBoxChecked();
    dlg->Show();
  }

  SUBCASE("Test dialog without buttons")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(
      std::vector <wxExTestItem> {
        {"string1"},
        {"string2"}},
      wxExWindowData().Button(0).Title("no buttons"));
    dlg->Show();
  }

  SUBCASE("Test dialog without items")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(
      std::vector <wxExTestItem>(),
      wxExWindowData().Title("no items"));
    dlg->Show();
  }
  
  SUBCASE("Test dialog with empty items")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(
      std::vector <wxExTestItem> {{}, {}, {}},
      wxExWindowData().Title("empty items"));
    dlg->Show();
  }
}
