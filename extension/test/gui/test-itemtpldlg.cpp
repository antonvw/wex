////////////////////////////////////////////////////////////////////////////////
// Name:      test-itemtpldlg.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h>
#include <wx/extension/item.h>
#include <wx/extension/itemtpldlg.h>
#include <wx/extension/managedframe.h>
#include "test.h"

class wxExTestItem : public wxExItem
{
public:
  wxExTestItem(): wxExItem() {;};
  wxExTestItem(const wxString& label, 
    const wxString& value = wxEmptyString, const wxString& page = wxEmptyString)
    : wxExItem(label, value, page) {;};
};

class wxExOtherTestItem : public wxExItem
{
public:
  wxExOtherTestItem(const wxString& label)
    : wxExItem(label, wxEmptyString, wxEmptyString) {;};
};

void fixture::testItemTemplateDialog()
{
  const std::vector<wxString> titles {
    "ITEM_AUINOTEBOOK",
    "ITEM_CHOICEBOOK",
    "ITEM_LISTBOOK",
    "ITEM_NOTEBOOK",
    "ITEM_SIMPLEBOOK",
    "ITEM_TOOLBOOK",
    "ITEM_TREEBOOK"};
  
  CPPUNIT_ASSERT(titles.size() == (wxExItemTemplateDialog<wxExTestItem>::ITEM_TREEBOOK
     - wxExItemTemplateDialog<wxExTestItem>::ITEM_AUINOTEBOOK + 1)); 
  
  // Test dialog using notebook with pages.
  for (
    int style = wxExItemTemplateDialog<wxExTestItem>::ITEM_AUINOTEBOOK; 
    style <= wxExItemTemplateDialog<wxExTestItem>::ITEM_TREEBOOK;
    style++)
  {
    wxImageList* il = nullptr;
    
    if (style == wxExItemTemplateDialog<wxExTestItem>::ITEM_TOOLBOOK)
    {
      const wxSize imageSize(32, 32);

      il = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
      
      il->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
    }
    
    wxExItemTemplateDialog<wxExTestItem>* dlg = new wxExItemTemplateDialog<wxExTestItem>(
      m_Frame, 
      std::vector <wxExTestItem> {
        wxExTestItem("string1", "nice", "page0:0"),
        wxExTestItem("string2", "", "page0:1"),
        wxExTestItem("string1", "other", "page1"),
        wxExTestItem("string2", "", "page1"),
        wxExTestItem("string1", "", "page2")},
      titles[style - wxExItemTemplateDialog<wxExTestItem>::ITEM_AUINOTEBOOK],
      0,
      1,
      wxOK | wxCANCEL | wxAPPLY,
      wxID_ANY,
      style,
      il);
      
    dlg->ForceCheckBoxChecked();
    dlg->Show();
    
#if wxCHECK_VERSION(3,1,0)
    CPPUNIT_ASSERT(dlg->GetItem("string1").GetValue() == "nice");
    CPPUNIT_ASSERT(dlg->GetItem("string1", "page0").GetValue() == "nice");
    CPPUNIT_ASSERT(dlg->GetItem("string1", "page1").GetValue() == "other");
    CPPUNIT_ASSERT(dlg->SetItemValue("string1", "xxx", "page1"));
    CPPUNIT_ASSERT(dlg->GetItem("string1").GetValue() == "nice");
    CPPUNIT_ASSERT(dlg->GetItem("string1", "page1").GetValue() == "xxx");
#endif

    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
  }
  
  // Test dialog without buttons.
  wxExItemTemplateDialog<wxExTestItem>* dlg0 = new wxExItemTemplateDialog<wxExTestItem>(m_Frame, 
    std::vector <wxExTestItem> {
      wxExTestItem("fruit", "apple", ""),
      wxExTestItem("more fruit", "citron", "")},
    "labels, no buttons", 0, 1, 0);
  
  CPPUNIT_ASSERT( dlg0->GetItem("fruit").GetLabel() == "fruit");
  CPPUNIT_ASSERT( dlg0->GetItemValue("fruit") == "apple");
  CPPUNIT_ASSERT( dlg0->GetItemValue("fruit", "xxx").IsNull());
  CPPUNIT_ASSERT( dlg0->GetItem("xxx").GetLabel().empty());
  CPPUNIT_ASSERT( dlg0->GetItemValue("yyy").IsNull());
  // asserts in 3.0
#if wxCHECK_VERSION(3,1,0)
  CPPUNIT_ASSERT( dlg0->SetItemValue("fruit", "strawberry"));
  CPPUNIT_ASSERT(!dlg0->SetItemValue("fruit", "blueberry", "xxxx"));
  CPPUNIT_ASSERT( dlg0->GetItemValue("fruit") == "strawberry");
#endif
  
  dlg0->Show();

  // Test dialog with empty pages (and no buttons).
  wxExItemTemplateDialog<wxExTestItem>* dlg1 = new wxExItemTemplateDialog<wxExTestItem>(m_Frame, 
    std::vector <wxExTestItem> {
      wxExTestItem("string1"),
      wxExTestItem("string2")},
    "empty pages, no buttons", 0, 1, 0);
  dlg1->Show();

  // Using other items.
  wxExItemTemplateDialog<wxExOtherTestItem>* dlg2 = new wxExItemTemplateDialog<wxExOtherTestItem>(m_Frame, 
    std::vector <wxExOtherTestItem> {
      wxExOtherTestItem("string1"),
      wxExOtherTestItem("string2")},
    "empty pages");
  dlg2->Show();
  
  // Test dialog without items.
  wxExItemTemplateDialog<wxExTestItem>* dlg3 = new wxExItemTemplateDialog<wxExTestItem>(m_Frame, 
    std::vector <wxExTestItem>(),
    "no items");
  dlg3->Show();
  
  // Test dialog with empty items.
  wxExItemTemplateDialog<wxExTestItem>* dlg4 = new wxExItemTemplateDialog<wxExTestItem>(m_Frame, 
    std::vector <wxExTestItem> {
      wxExTestItem(), wxExTestItem(), wxExTestItem()},
    "empty items");
  dlg4->Show();
}
