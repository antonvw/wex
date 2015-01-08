////////////////////////////////////////////////////////////////////////////////
// Name:      test-statusbar.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/statusbar.h>
#include "test.h"

void wxExGuiTestFixture::testStatusBar()
{
  wxExFrame* frame = (wxExFrame*)wxTheApp->GetTopWindow();
  // Real testing already done in testFrame

  std::vector<wxExStatusBarPane> panes;
  panes.push_back(wxExStatusBarPane());
  panes.push_back(wxExStatusBarPane("panex"));
  panes.push_back(wxExStatusBarPane("paney"));
  panes.push_back(wxExStatusBarPane("panez"));

  //wxExStatusBar* sb = 
  new wxExStatusBar(frame);
}
