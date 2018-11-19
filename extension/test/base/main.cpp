////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"

IMPLEMENT_APP_NO_MAIN(wex::test_app);

int main (int argc, char* argv[])
{
  return wex::testmain(argc, argv, new wex::test_app());
}  

TEST_CASE( "wex::test_app" ) 
{
  wxASSERT( 1 == 0 ); // to test OnAssertFailure
  
  wxGetApp().get_catalog_dir();
  wxGetApp().get_locale().IsOk();
}
