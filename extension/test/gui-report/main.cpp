////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension report cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
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
    
  frame->Show(true);

  wxLog::SetActiveTarget(new wxLogStderr());
    
  CppUnit::TextUi::TestRunner runner;

  wxExTestSuite* suite = new wxExTestSuite;

  runner.addTest(suite);
  m_Success = runner.run("", false);
  
  return true;
}

int wxExTestApp::OnRun()
{
  return !m_Success;
}
