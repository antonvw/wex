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
#include <wx/extension/managedframe.h>
#include "test.h"

wxExStatusBar* wxExGuiTestFixture::m_StatusBar = NULL;

wxExGuiTestFixture::wxExGuiTestFixture()
  : wxExTestFixture() 
  , m_Frame((wxExManagedFrame *)wxTheApp->GetTopWindow())
{
  // Create the global lexers object, 
  // it should be present in ~/.wxex-test-gui
  // (depending on platform, configuration).
  wxExLexers::Get();
  
  wxConfigBase::Get()->Write(_("vi mode"), true);
} 

wxExGuiTestFixture::~wxExGuiTestFixture()
{
  // Remove files.
  (void)remove("test-ex.txt");
  (void)remove("test.hex");
}
