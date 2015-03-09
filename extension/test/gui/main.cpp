////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015
////////////////////////////////////////////////////////////////////////////////

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/TestRunner.h>
#include <wx/cmdline.h> // for wxCmdLineParser
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

void wxExTestApp::OnInitCmdLine(wxCmdLineParser& parser)
{
  wxExApp::OnInitCmdLine(parser);

  parser.AddParam(
    "test",
    wxCMD_LINE_VAL_STRING,
    wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE);
}

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( wxExGuiTestFixture );

int wxExTestApp::OnRun()
{
  std::string name = "";
  
  if (argc > 1)
  {
    name = argv[1];
  }
      
  try
  {
    CppUnit::Test* suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
    
    CppUnit::TextUi::TestRunner runner;
    runner.addTest(suite);
    
    const bool success = runner.run(name, false);
  
    exit(!success);
  }
  catch (std::invalid_argument e)
  {
    printf("invalid test: %s\n", name.c_str());
    return 0;
  }
  
  return 0;
}
