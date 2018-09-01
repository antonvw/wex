////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"

IMPLEMENT_APP_NO_MAIN(wxExTestApp);

int main (int argc, char* argv[])
{
  return wxExTestMain(argc, argv, new wxExTestApp());
}  

TEST_CASE( "wxExTestApp" ) 
{
  wxASSERT( 1 == 0 ); // to test OnAssertFailure
  
  wxGetApp().GetCatalogDir();
  wxGetApp().GetLocale().IsOk();
}
