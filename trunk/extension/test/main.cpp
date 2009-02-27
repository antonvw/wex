/******************************************************************************\
* File:          main.cpp
* Purpose:       main for wxextension cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       za 17 jan 2009 11:51:20 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include "test.h"

#ifndef APP_TEST
int main (int argc, char* argv[]) 
{
  CppUnit::TextUi::TestRunner runner;
  
  exTestSuite* suite = new exTestSuite;
  
  runner.addTest(suite);
  runner.run();
  
  return 0;
}
#else
// main is not compiled, so we can use exApp
IMPLEMENT_APP(exTestApp)

bool exTestApp::OnInit()
{
  SetAppName("exTestApp");

  exApp::OnInit();

  exFrame *frame = new exFrame(NULL, wxID_ANY, "exTestApp");
  frame->Show(true);

  SetTopWindow(frame);

  CppUnit::TextUi::TestRunner runner;

  exTestSuite* suite = new exTestSuite;

  runner.addTest(suite);
  runner.run();

  return true;
}
#endif
