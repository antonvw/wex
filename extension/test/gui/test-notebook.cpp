////////////////////////////////////////////////////////////////////////////////
// Name:      test-notebook.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/notebook.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExNotebook")
{
  wxExNotebook* notebook = new wxExNotebook();
  AddPane(GetFrame(), notebook);
  
  wxWindow* page1 = new wxWindow(GetFrame(), wxID_ANY);
  wxWindow* page2 = new wxWindow(GetFrame(), wxID_ANY);
  wxWindow* page3 = new wxWindow(GetFrame(), wxID_ANY);
  wxWindow* page4 = new wxWindow(GetFrame(), wxID_ANY);
  wxWindow* page5 = new wxWindow(GetFrame(), wxID_ANY);
  
  // Test AddPage. 
  REQUIRE(notebook->AddPage(page1, "key1") != nullptr);
  REQUIRE(notebook->AddPage(page2, "key2") != nullptr);
  REQUIRE(notebook->AddPage(page3, "key3") != nullptr);
  // pages: 0,1,2 keys: key1, key2, key3 pages page1,page2,page3.
  
  // Test GetKeyByPage, GetPageByKey, GetPageIndexByKey.
  REQUIRE(notebook->GetKeyByPage(page1) == "key1");
  REQUIRE(notebook->GetPageByKey("key1") == page1);
  REQUIRE(notebook->GetPageIndexByKey("key1") == 0);
  REQUIRE(notebook->GetPageIndexByKey("xxx") == wxNOT_FOUND);
  
  // Test SetPageText.
  REQUIRE(notebook->SetPageText("key1", "keyx", "hello"));
  REQUIRE(notebook->GetPageByKey("keyx") == page1);
  // pages: 0,1,2 keys: keyx, key2, key3 pages page1, page2,page3.
  REQUIRE(notebook->GetPageIndexByKey("key1") == wxNOT_FOUND);
  
  // Test DeletePage.
  REQUIRE(notebook->DeletePage("keyx"));
  REQUIRE(notebook->GetPageByKey("keyx") == nullptr);
  REQUIRE(notebook->DeletePage("key2"));
  REQUIRE(!notebook->DeletePage("xxx"));
  // pages: 0 keys: key3 pages:page3.

  // Test InsertPage.
  REQUIRE(notebook->InsertPage(0, page4, "KEY1") != nullptr);
  REQUIRE(notebook->InsertPage(0, page5, "KEY0") != nullptr);
  // pages: 0,1,2 keys: KEY0, KEY1, key3 pages: page5,page4,page3.
  REQUIRE(notebook->GetPageIndexByKey("KEY0") == 0);
  REQUIRE(notebook->GetPageIndexByKey("KEY1") == 1);
  
  // Test SetSelection.
  REQUIRE(notebook->SetSelection("KEY1") == page4);
  REQUIRE(notebook->GetCurrentPage() == "KEY1");
  REQUIRE(notebook->SetSelection("key3") == page3);
  REQUIRE(notebook->GetCurrentPage() == "key3");
  REQUIRE(notebook->SetSelection("XXX") == nullptr);
  REQUIRE(notebook->GetCurrentPage() == "key3");

  // Test ChangeSelection.
  REQUIRE(notebook->ChangeSelection("KEY1") == "key3");
  REQUIRE(notebook->GetCurrentPage() == "KEY1");
  REQUIRE(notebook->ChangeSelection("key3") == "KEY1");
  REQUIRE(notebook->GetCurrentPage() == "key3");
  REQUIRE(notebook->ChangeSelection("XXX") == std::string());
  REQUIRE(notebook->GetCurrentPage() == "key3");

  // Prepare next test, delete all pages.
  REQUIRE(notebook->DeletePage("KEY0"));
  REQUIRE(notebook->DeletePage("KEY1"));
  REQUIRE(notebook->DeletePage("key3"));
  REQUIRE(notebook->GetPageCount() == 0);
  
  // Test ForEach.
  wxExSTC* stc_x = new wxExSTC(std::string("hello stc"));
  wxExSTC* stc_y = new wxExSTC(std::string("hello stc"));
  wxExSTC* stc_z = new wxExSTC(std::string("hello stc"));
  
  REQUIRE(notebook->AddPage(stc_x, "key1") != nullptr);
  REQUIRE(notebook->AddPage(stc_y, "key2") != nullptr);
  REQUIRE(notebook->AddPage(stc_z, "key3") != nullptr);
  
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_STC_SET_LEXER));
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_STC_SET_LEXER_THEME));
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_STC_SYNC));
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_CONFIG_GET));
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_SAVE));
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_CLOSE_OTHERS));
  REQUIRE(notebook->GetPageCount() == 1);
  REQUIRE(notebook->ForEach<wxExSTC>(ID_ALL_CLOSE));
  REQUIRE(notebook->GetPageCount() == 0);
  
  // Test Rearrange.
  notebook->Rearrange(wxLEFT);
  notebook->Rearrange(wxBOTTOM);
  
  // Test Split.
  wxWindow* pagev = new wxWindow(GetFrame(), wxID_ANY);
  REQUIRE( notebook->AddPage(pagev, "keyv") != nullptr);
  // split having only one page
  REQUIRE( notebook->Split("keyv", wxRIGHT));
  wxWindow* pagew = new wxWindow(GetFrame(), wxID_ANY);
  REQUIRE( notebook->AddPage(pagew, "keyw") != nullptr);
  // split using incorrect key
  REQUIRE(!notebook->Split("err", wxRIGHT));
  REQUIRE( notebook->Split("keyv", wxRIGHT));
  REQUIRE(notebook->GetPageCount() == 2);
}
