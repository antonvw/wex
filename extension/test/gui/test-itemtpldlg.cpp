////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemtpldlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/item.h>
#include <wx/extension/itemtpldlg.h>
#include <wx/extension/report/defs.h>
#include <wx/extension/managedframe.h>
#include "test.h"

class wxExTestItem : public wxExItem
{
public:
  wxExTestItem(): wxExItem() {;};
  wxExTestItem(const wxString& label, 
    const wxString& value = wxEmptyString)
    : wxExItem(label, value) {;};
  void SetDialog(wxExItemTemplateDialog<wxExTestItem>* dlg) {;};
};

class wxExOtherTestItem : public wxExItem
{
public:
  wxExOtherTestItem(const wxString& label)
    : wxExItem(label, wxEmptyString) {;};
  void SetDialog(wxExItemTemplateDialog<wxExOtherTestItem>* dlg) {;};
};

TEST_CASE("wxExItemTemplateDialog", "[item]")
{
  wxExItem::UseConfig(false);

  wxExItemTemplateDialog<wxExTestItem>* dlg0 = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
    std::vector <wxExTestItem> {
      wxExTestItem("fruit", "apple"),
      wxExTestItem("string1"),
      wxExTestItem("string2"),
      wxExTestItem("more fruit", "citron")},
    "3 columns", 0, 3);
  
  dlg0->Show();
  
  REQUIRE( dlg0->GetItem("fruit").GetLabel() == "fruit");
  REQUIRE( dlg0->GetItemValue("fruit") == "apple");
  REQUIRE( dlg0->GetItemValue("xxx").IsNull());
  REQUIRE( dlg0->GetItem("xxx").GetLabel().empty());
  REQUIRE( dlg0->GetItemValue("yyy").IsNull());
  
  // asserts in 3.0
#if wxCHECK_VERSION(3,1,0)
  REQUIRE( dlg0->SetItemValue("fruit", "strawberry"));
  REQUIRE(!dlg0->SetItemValue("xxx", "blueberry"));
  REQUIRE( dlg0->GetItemValue("fruit") == "strawberry");
#endif
  
  dlg0->ForceCheckBoxChecked();
  
  // Test dialog without buttons.
  wxExItemTemplateDialog<wxExTestItem>* dlg1 = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
    std::vector <wxExTestItem> {
      wxExTestItem("string1"),
      wxExTestItem("string2")},
    "no buttons", 0, 1, 0);
  dlg1->Show();

  // Test dialog without items.
  wxExItemTemplateDialog<wxExTestItem>* dlg2 = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
    std::vector <wxExTestItem>(),
    "no items");
  dlg2->Show();
  
  // Test dialog with empty items.
  wxExItemTemplateDialog<wxExTestItem>* dlg3 = new wxExItemTemplateDialog<wxExTestItem>(GetFrame(), 
    std::vector <wxExTestItem> {
      wxExTestItem(), wxExTestItem(), wxExTestItem()},
    "empty items");
  dlg3->Show();
}
