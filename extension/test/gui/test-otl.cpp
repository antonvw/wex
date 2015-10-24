////////////////////////////////////////////////////////////////////////////////
// Name:      test-otl.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/otl.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testOTL()
{
#if wxExUSE_OTL
  // Ensure we have a database and a table.
  system("mysql test < otl-create.sql");
  

  wxConfigBase* config = wxConfigBase::Get(false);
  config->Write(_("Datasource"), "Test");
  config->Write(_("User"), "");
  config->Write(_("Password"), "");
  
  wxExOTL otl;
  
  CPPUNIT_ASSERT(!otl.Datasource().empty());
  
  otl.Logon();
  
  if (!otl.IsConnected())
  {
    CPPUNIT_ASSERT( otl.Query("select * from one") == 0);
    CPPUNIT_ASSERT(!otl.Logoff());
  }
  else
  {
    CPPUNIT_ASSERT( otl.Query("select * from one") == 9);
    CPPUNIT_ASSERT( otl.Logoff());
  }
#endif
}
