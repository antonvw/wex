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
#include "test.h"

void wxExGuiTestFixture::testConfigDialog()
{
  // Test config dialog using notebook with pages.
  std::vector <wxExConfigItem> items;
  
  wxExConfigItem ci1("string1", "test", "page0");
  items.push_back(ci1);
  wxExConfigItem ci2("string2", "test", "page0");
  items.push_back(ci2);
  
  wxExConfigDialog dlg(wxTheApp->GetTopWindow(), items);
  
  dlg.ForceCheckBoxChecked();
  dlg.Show();
  dlg.Reload();
  
  // Test config dialog without pages.
  std::vector <wxExConfigItem> items2;
  items2.push_back(wxExConfigItem("string1"));
  
  wxExConfigDialog dlg2(wxTheApp->GetTopWindow(), items2);
  dlg2.Show();
  
  // Test config dialog without items.
  wxExConfigDialog dlg3(wxTheApp->GetTopWindow(), std::vector <wxExConfigItem>());
  dlg3.Show();
}
