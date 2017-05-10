////////////////////////////////////////////////////////////////////////////////
// Name:      test-otl.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/managedframe.h>
#include "test.h"
#include <wx/extension/otl.h>

TEST_CASE("wxExOTL")
{
#if wxExUSE_OTL
  // Ensure we have a database and a table.
  system("mysql test < otl-create.sql");

  wxConfigBase* config = wxConfigBase::Get(false);
  config->Write(_("Datasource"), "Test");
  config->Write(_("User"), "");
  config->Write(_("Password"), "");
  
  wxExOTL otl;
  
  REQUIRE( otl.VersionInfo().GetMajor() > 0);
  REQUIRE(!otl.Datasource().empty());
  
  otl.Logon(wxExWindowData().Button(0));
  
  if (!otl.IsConnected())
  {
    REQUIRE( otl.Query("select * from one") == 0);
    REQUIRE(!otl.Logoff());
  }
  else
  {
    REQUIRE( otl.Query("select * from one") == 9);
    REQUIRE( otl.Logoff());
  }
#endif
}
