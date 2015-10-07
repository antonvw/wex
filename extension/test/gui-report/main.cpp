////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <cppunit/extensions/HelperMacros.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( fixture );
