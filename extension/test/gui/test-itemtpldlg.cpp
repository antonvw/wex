////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemtpldlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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
  wxExTestItem(const wxString& label, const wxString& value = wxEmptyString)
    : wxExItem(label, value) {;};
  wxExTestItem(const wxString& label, wxExItemType type)
    : wxExItem(label, type) {;};
  void SetDialog(wxExItemTemplateDialog<wxExTestItem>* dlg) {;};
};

TEST_CASE("wxExItemTemplateDialog", "[item]")
{
  wxExItem::UseConfig(false);

  SECTION("Basic")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
      std::vector <wxExTestItem> {
        wxExTestItem("fruit", "apple"),
        wxExTestItem("button", ITEM_BUTTON),
        wxExTestItem("string1"),
        wxExTestItem("string2"),
        wxExTestItem("more fruit", "citron")},
      "3 columns", 0, 3);
    
    REQUIRE( wxExTestItem("test", ITEM_BUTTON).GetType() == ITEM_BUTTON);
    
    REQUIRE(!dlg->BindButton(wxExTestItem()));
    REQUIRE(!dlg->BindButton(wxExTestItem("test", ITEM_COMBOBOX)));
    REQUIRE(!dlg->BindButton(wxExTestItem("test", ITEM_BUTTON))); // not yet laid out0
    REQUIRE(!dlg->BindButton(wxExTestItem("test", ITEM_COMBOBOX_DIR))); // same

    REQUIRE( dlg->BindButton(dlg->GetItem("button")));
    
    dlg->Show();
    
    REQUIRE( dlg->GetItem("fruit").GetLabel() == "fruit");
    REQUIRE( dlg->GetItemValue("fruit") == "apple");
    REQUIRE( dlg->GetItemValue("xxx").IsNull());
    REQUIRE( dlg->GetItem("xxx").GetLabel().empty());
    REQUIRE( dlg->GetItemValue("yyy").IsNull());
    
    // asserts in 3.0
#if wxCHECK_VERSION(3,1,0)
    REQUIRE( dlg->SetItemValue("fruit", "strawberry"));
    REQUIRE(!dlg->SetItemValue("xxx", "blueberry"));
    REQUIRE( dlg->GetItemValue("fruit") == "strawberry");
#endif
    
    dlg->ForceCheckBoxChecked();
  }

  SECTION("Test dialog with checkbox item")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
      std::vector <wxExTestItem> {
        wxExTestItem("checkbox", ITEM_CHECKBOX)},
      "checkbox items");

    dlg->ForceCheckBoxChecked();
    dlg->Show();
  }

  SECTION("Test dialog without buttons")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
      std::vector <wxExTestItem> {
        wxExTestItem("string1"),
        wxExTestItem("string2")},
      "no buttons", 0, 1, 0);
    dlg->Show();
  }

  SECTION("Test dialog without items")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
      std::vector <wxExTestItem>(),
      "no items");
    dlg->Show();
  }
  
  SECTION("Test dialog with empty items")
  {
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
      std::vector <wxExTestItem> {
        wxExTestItem(), wxExTestItem(), wxExTestItem()},
      "empty items");
    dlg->Show();
  }
}
