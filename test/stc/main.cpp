////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "test.h"

IMPLEMENT_APP_NO_MAIN(wex::test::stc_app);

int main(int argc, char* argv[])
{
  return wex::test::main(argc, argv, new wex::test::stc_app());
}
