////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016
////////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_RUNNER

#include "../catch.hpp"
#include "../test.h"

IMPLEMENT_APP_NO_MAIN(wxExTestApp);

int main (int argc, char* argv[])
{
  Catch::Session session; // There must be exactly once instance

  int returnCode = session.applyCommandLine( argc, (const char **)argv );
  if( returnCode != 0 ) // Indicates a command line error
    return returnCode;
  
  wxApp::SetInstance( new wxExTestApp() );
  wxEntryStart( argc, argv );

  wxGetApp().OnInit();
  
  int ret = session.run();

  wxGetApp().OnExit();
  
  return ret;
}  

TEST_CASE( "wxExTestApp" ) 
{
  REQUIRE(!wxGetApp().GetCatalogDir().empty());
  REQUIRE(!wxGetApp().GetLocale().GetName().empty());
}
