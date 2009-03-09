/******************************************************************************\
* File:          main.cpp
* Purpose:       main for wxfiletool cpp unit testing
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
* Created:       zo 08 mrt 2009 16:16:29 CET
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include "test.h"

IMPLEMENT_APP(ftTestApp)

bool ftTestApp::OnInit()
{
  SetAppName("ftTestApp");

  exApp::OnInit();

  ftFrame *frame = new ftFrame(NULL, wxID_ANY, "ftTestApp");
  frame->Show(true);

  SetTopWindow(frame);

  CppUnit::TextUi::TestRunner runner;

  ftTestSuite* suite = new ftTestSuite;

  runner.addTest(suite);
  runner.run();

  return true;
}
