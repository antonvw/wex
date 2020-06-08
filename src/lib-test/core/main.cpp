////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"

IMPLEMENT_APP_NO_MAIN(wex::test::app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::app());
}

TEST_CASE("wex::test::app")
{
  wxGetApp().get_catalog_dir();
  wxGetApp().get_locale().IsOk();
}
