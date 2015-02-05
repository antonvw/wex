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

void wxExGuiTestFixture::testConfigDialog()
{
  // Test config dialog using notebook with pages.
  const std::vector <wxExConfigItem> items{
    wxExConfigItem("string1", "test", "page0"),
    wxExConfigItem("string2", "test", "page1")};
    
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
      items,
      "title",
      0,
      1,
      wxOK | wxCANCEL,
      wxID_ANY,
      style,
      il);
      
    dlg->ForceCheckBoxChecked();
    dlg->Show();
    dlg->Reload();
    dlg->Destroy();
  }
  
  // Test config dialog without pages.
  const std::vector <wxExConfigItem> items2{wxExConfigItem("string1")};
  wxExConfigDialog dlg2(m_Frame, items2);
  dlg2.Show();
  
  // Test config dialog without items.
  wxExConfigDialog dlg3(m_Frame, std::vector <wxExConfigItem>());
  dlg3.Show();
}
