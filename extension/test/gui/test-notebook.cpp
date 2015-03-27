////////////////////////////////////////////////////////////////////////////////
// Name:      test-notebook.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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

void fixture::testNotebook()
{
  wxExNotebook* notebook = new wxExNotebook(m_Frame, NULL);
  wxWindow* page1 = new wxWindow(m_Frame, wxID_ANY);
  wxWindow* page2 = new wxWindow(m_Frame, wxID_ANY);
  wxWindow* page3 = new wxWindow(m_Frame, wxID_ANY);
  wxWindow* page4 = new wxWindow(m_Frame, wxID_ANY);
  wxWindow* page5 = new wxWindow(m_Frame, wxID_ANY);
  
  // Test AddPage. 
  CPPUNIT_ASSERT(notebook->AddPage(page1, "key1") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(page2, "key2") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(page3, "key3") != NULL);
  // pages: 0,1,2 keys: key1, key2, key3 pages page1,page2,page3.
  
  // Test GetKeyByPage, GetPageByKey, GetPageIndexByKey.
  CPPUNIT_ASSERT(notebook->GetKeyByPage(page1) == "key1");
  CPPUNIT_ASSERT(notebook->GetPageByKey("key1") == page1);
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("key1") == 0);
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("xxx") == wxNOT_FOUND);
  
  // Test SetPageText.
  CPPUNIT_ASSERT(notebook->SetPageText("key1", "keyx", "hello"));
  CPPUNIT_ASSERT(notebook->GetPageByKey("keyx") == page1);
  // pages: 0,1,2 keys: keyx, key2, key3 pages page1, page2,page3.
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("key1") == wxNOT_FOUND);
  
  // Test DeletePage.
  CPPUNIT_ASSERT(notebook->DeletePage("keyx"));
  CPPUNIT_ASSERT(notebook->GetPageByKey("keyx") == NULL);
  CPPUNIT_ASSERT(notebook->DeletePage("key2"));
  CPPUNIT_ASSERT(!notebook->DeletePage("xxx"));
  // pages: 0 keys: key3 pages:page3.

  // Test InsertPage.
  CPPUNIT_ASSERT(notebook->InsertPage(0, page4, "KEY1") != NULL);
  CPPUNIT_ASSERT(notebook->InsertPage(0, page5, "KEY0") != NULL);
  // pages: 0,1,2 keys: KEY0, KEY1, key3 pages: page5,page4,page3.
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("KEY0") == 0);
  CPPUNIT_ASSERT(notebook->GetPageIndexByKey("KEY1") == 1);
  
  // Test SetSelection.
  CPPUNIT_ASSERT(notebook->SetSelection("KEY1") == page4);
  CPPUNIT_ASSERT(notebook->SetSelection("key3") == page3);
  CPPUNIT_ASSERT(notebook->SetSelection("XXX") == NULL);
  
  // Prepare next test, delete all pages.
  CPPUNIT_ASSERT(notebook->DeletePage("KEY0"));
  CPPUNIT_ASSERT(notebook->DeletePage("KEY1"));
  CPPUNIT_ASSERT(notebook->DeletePage("key3"));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 0);
  
  // Test ForEach. ForEach expects wxExSTC pages.  
  wxExSTC* stc_x = new wxExSTC(m_Frame, "hello stc");
  wxExSTC* stc_y = new wxExSTC(m_Frame, "hello stc");
  wxExSTC* stc_z = new wxExSTC(m_Frame, "hello stc");
  
  CPPUNIT_ASSERT(notebook->AddPage(stc_x, "key1") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(stc_y, "key2") != NULL);
  CPPUNIT_ASSERT(notebook->AddPage(stc_z, "key3") != NULL);
  
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_SET_LEXER));
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_SET_LEXER_THEME));
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_CONFIG_GET));
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_CLOSE_OTHERS));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 1);
  CPPUNIT_ASSERT(notebook->ForEach(ID_ALL_STC_CLOSE));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 0);
  
  // Test Split.
  wxWindow* pagev = new wxWindow(m_Frame, wxID_ANY);
  CPPUNIT_ASSERT( notebook->AddPage(pagev, "keyv") != NULL);
  // split having only one page
  CPPUNIT_ASSERT( notebook->Split("keyv", wxRIGHT));
  wxWindow* pagew = new wxWindow(m_Frame, wxID_ANY);
  CPPUNIT_ASSERT( notebook->AddPage(pagew, "keyw") != NULL);
  // split using incorrect key
  CPPUNIT_ASSERT(!notebook->Split("err", wxRIGHT));
  CPPUNIT_ASSERT( notebook->Split("keyv", wxRIGHT));
  CPPUNIT_ASSERT(notebook->GetPageCount() == 2);
}
