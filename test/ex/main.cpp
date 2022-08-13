////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

IMPLEMENT_APP_NO_MAIN(wex::test::ex);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::ex());
}
