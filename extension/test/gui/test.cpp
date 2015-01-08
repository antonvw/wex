////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/lexers.h>
#include "test.h"

wxExGuiTestFixture::wxExGuiTestFixture()
  : wxExTestFixture() 
{
} 

wxExGuiTestFixture::~wxExGuiTestFixture()
{
  // Remove files.
  remove("test-ex.txt");
  remove("test.hex");
}

void wxExGuiTestFixture::setUp()
{
  // Create the global lexers object, 
  // it should be present in ~/.wxex-test-gui
  // (depending on platform, configuration).
  wxExLexers::Get();
  
  wxConfigBase::Get()->Write(_("vi mode"), true);
}

void wxExGuiTestFixture::tearDown() 
{
  wxExTestFixture::tearDown();
}
