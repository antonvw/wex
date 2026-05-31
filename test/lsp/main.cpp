////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>

IMPLEMENT_APP_NO_MAIN(wex::test::app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::app());
}
