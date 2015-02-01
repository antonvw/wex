////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include <wx/stdpaths.h>
#include <wx/extension/managedframe.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");

  SetWorkingDirectory();
  SetEnvironment(wxStandardPaths::Get().GetUserDataDir());
  
  if (!wxExApp::OnInit())
  {
    return false;
  }
  
  new wxExManagedFrame(NULL, wxID_ANY, wxTheApp->GetAppDisplayName());
    
  wxLogStatus(GetCatalogDir());
  wxLogStatus(GetLocale().GetLocale());
  
  return true;
}

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( wxExGuiTestFixture );

int wxExTestApp::OnRun()
{
  // Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();

  CppUnit::TextUi::TestRunner runner;
  runner.addTest(suite);
  
  const bool success = runner.run("", false);
  
  exit(!success);
  
  return 0;
}
