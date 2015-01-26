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
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include <wx/stdpaths.h>
#include <wx/sysopt.h>
#include <wx/extension/report/frame.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

FrameWithHistory::FrameWithHistory(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  size_t maxFiles,
  size_t maxProjects,
  int style)
  : wxExFrameWithHistory(parent, id, title, maxFiles, maxProjects, style)
{
  wxExLexer lexer("cpp");
  m_Report = new wxExListViewFileName(
    this, 
    wxExListViewFileName::LIST_KEYWORD,
    wxID_ANY,
    &lexer);
}

wxExListViewFileName* FrameWithHistory::Activate(
  wxExListViewFileName::wxExListType list_type, 
  const wxExLexer* lexer)
{
  return m_Report;
}

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui-report");
  wxSystemOptions::SetOption("gtk.desktop", "GNOME");
  
  if (!wxExApp::OnInit())
  {
    return false;
  }

  FrameWithHistory* frame = new 
    FrameWithHistory(NULL, wxID_ANY, wxTheApp->GetAppDisplayName());
    
  return true;
}

// Registers the fixture into the 'registry'
CPPUNIT_TEST_SUITE_REGISTRATION( wxExGuiReportTestFixture );

int wxExTestApp::OnRun()
{
  CppUnit::TextUi::TestRunner runner;

  const wxString& old = SetWorkingDirectory();
  SetEnvironment(wxStandardPaths::Get().GetUserDataDir());
  
  // Get the top level suite from the registry
  CppUnit::Test *suite = CppUnit::TestFactoryRegistry::getRegistry().makeTest();
  
  runner.addTest(suite);
  
  const bool success = runner.run("", false);
  
  wxSetWorkingDirectory(old);
  
  return !success;
}
