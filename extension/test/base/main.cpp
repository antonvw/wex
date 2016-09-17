////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016
////////////////////////////////////////////////////////////////////////////////

#include <string.h>

#include "../catch.hpp"
#include "../test.h"

IMPLEMENT_APP_NO_MAIN(wxExTestApp);

int main (int argc, char* argv[])
{
  return wxExTestMain(argc, argv, new wxExTestApp(), false);
}  

TEST_CASE( "wxExTestApp" ) 
{
  wxASSERT( 1 == 0 ); // to test OnAssertFailure
  REQUIRE(!wxGetApp().GetCatalogDir().empty());
  REQUIRE( wxGetApp().GetLocale().IsOk());
  INFO( wxGetApp().GetLocale().GetLanguage());
}
