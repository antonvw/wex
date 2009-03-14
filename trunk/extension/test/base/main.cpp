/******************************************************************************\
* File:          main.cpp
* Purpose:       main for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: main.cpp 406 2009-02-27 18:47:53Z antonvw $
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include "test.h"

int main (int argc, char* argv[]) 
{
  CppUnit::TextUi::TestRunner runner;
  
  exTestSuite* suite = new exTestSuite;
  
  runner.addTest(suite);
  runner.run();
  
  return 0;
}

