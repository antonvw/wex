////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#define CATCH_CONFIG_RUNNER

#include "../catch.hpp"
#include "../test.h"

int main (int argc, char* argv[])
{
  Catch::Session session; // There must be exactly once instance

  int returnCode = session.applyCommandLine( argc, argv );
  if( returnCode != 0 ) // Indicates a command line error
    return returnCode;

  SetWorkingDirectory();
  SetEnvironment(wxGetHomeDir() + "/.wxex-test-base");
  
  return session.run();
}  
