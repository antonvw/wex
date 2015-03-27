////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( fixture );
