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

void wxExGuiTestFixture::testConfigDialog()
{
  // Test config dialog using notebook with pages.
  const std::vector <wxExConfigItem>{
    wxExConfigItem("string1", "test", "page0"),
    wxExConfigItem("string2", "test", "page1")};
    
  for (int style = CONFIG_AUINOTEBOOK; i <= CONFIG_TREEBOOK; i++)
  {
    wxExConfigDialog dlg(
      m_Frame, 
      items,
      "title",
      0,
      1,
      wxOK | wxCANCEL,
      wxID_ANY,
      style);
    
    dlg.ForceCheckBoxChecked();
    dlg.Show();
    dlg.Reload();
  }
  
  // Test config dialog without pages.
  const std::vector <wxExConfigItem> items2{wxExConfigItem("string1")};
  wxExConfigDialog dlg2(m_Frame, items2);
  dlg2.Show();
  
  // Test config dialog without items.
  wxExConfigDialog dlg3(m_Frame, std::vector <wxExConfigItem>());
  dlg3.Show();
}
