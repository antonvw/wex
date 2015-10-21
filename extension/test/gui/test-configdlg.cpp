////////////////////////////////////////////////////////////////////////////////
// Name:      test-configdlg.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/configdlg.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testConfigDialog()
{
  wxExConfigDialog* dlg = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem> {
      wxExConfigItem("string1", "test1", "page0:0"),
      wxExConfigItem("string2", "test2", "page0:1"),
      wxExConfigItem("string3", "test3", "page1"),
      wxExConfigItem("string4", "test4", "page1"),
      wxExConfigItem("string5", "test5", "page2")},
    "config dialog", 0, 1, wxOK | wxCANCEL | wxAPPLY);
  dlg->ForceCheckBoxChecked();
  dlg->Show();
  dlg->Reload();
  
  wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
  wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
  
  // Test config dialog without pages.
  wxExConfigDialog* dlg1 = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem> {
      wxExConfigItem("string1", ""),
      wxExConfigItem("string2", "")},
    "no pages");
  dlg1->Show();
  
  // Test config dialog without items.
  wxExConfigDialog* dlg2 = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem>(),
    "no items");
  dlg2->Show();
  
  // Test config dialog with empty items.
  wxExConfigDialog* dlg3 = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem> {
      wxExConfigItem(), wxExConfigItem(), wxExConfigItem()},
    "empty items");
  dlg3->Show();
}
