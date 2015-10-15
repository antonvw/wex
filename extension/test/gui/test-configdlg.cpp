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
#include <wx/artprov.h>
#include <wx/extension/configdlg.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testConfigDialog()
{
  const std::vector<wxString> titles {
    "CONFIG_AUINOTEBOOK",
    "CONFIG_CHOICEBOOK",
    "CONFIG_LISTBOOK",
    "CONFIG_NOTEBOOK",
    "CONFIG_SIMPLEBOOK",
    "CONFIG_TOOLBOOK",
    "CONFIG_TREEBOOK"};
  
  CPPUNIT_ASSERT(titles.size() == (wxExConfigDialog::CONFIG_TREEBOOK
     - wxExConfigDialog::CONFIG_AUINOTEBOOK + 1)); 
  
  // Test config dialog using notebook with pages.
  for (
    int style = wxExConfigDialog::CONFIG_AUINOTEBOOK; 
    style <= wxExConfigDialog::CONFIG_TREEBOOK;
    style++)
  {
    wxImageList* il = NULL;
    
    if (style == wxExConfigDialog::CONFIG_TOOLBOOK)
    {
      const wxSize imageSize(32, 32);

      il = new wxImageList(imageSize.GetWidth(), imageSize.GetHeight());
      
      il->Add(wxArtProvider::GetIcon(wxART_INFORMATION, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_QUESTION, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_WARNING, wxART_OTHER, imageSize));
      il->Add(wxArtProvider::GetIcon(wxART_ERROR, wxART_OTHER, imageSize));
    }
    
    wxExConfigDialog* dlg = new wxExConfigDialog(
      m_Frame, 
      std::vector <wxExConfigItem> {
        wxExConfigItem("string1", "test1", _("page0") + ":0"),
        wxExConfigItem("string2", "test2", "page0:1"),
        wxExConfigItem("string3", "test3", "page1"),
        wxExConfigItem("string4", "test4", "page1"),
        wxExConfigItem("string5", "test5", "page2")},
      titles[style - wxExConfigDialog::CONFIG_AUINOTEBOOK],
      0,
      1,
      wxOK | wxCANCEL | wxAPPLY,
      wxID_ANY,
      style,
      il);
      
    dlg->ForceCheckBoxChecked();
    dlg->Show();
    dlg->Reload();
    
    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxAPPLY));
    wxPostEvent(dlg, wxCommandEvent(wxEVT_BUTTON, wxOK));
  }
  
  // Test config dialog without pages.
  wxExConfigDialog* dlg2 = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem> {
      wxExConfigItem("string1", ""),
      wxExConfigItem("string2", "")},
    "no pages");
  dlg2->Show();
  
  // Test config dialog without items.
  wxExConfigDialog* dlg3 = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem>(),
    "no items");
  dlg3->Show();
  
  // Test config dialog with empty items.
  wxExConfigDialog* dlg4 = new wxExConfigDialog(m_Frame, 
    std::vector <wxExConfigItem> {
      wxExConfigItem(), wxExConfigItem(), wxExConfigItem()},
    "empty items");
  dlg4->Show();
}
