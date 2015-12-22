////////////////////////////////////////////////////////////////////////////////
// Name:      test-filedlg.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/filedlg.h>
#include <wx/extension/file.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExFileDialog")
{
  wxExFile file;
  wxExFileDialog dlg(GetFrame(), &file);
  
  REQUIRE(dlg.ShowModalIfChanged(false) == wxID_OK);
}
