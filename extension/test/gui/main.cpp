////////////////////////////////////////////////////////////////////////////////
// Name:      main.cpp
// Purpose:   main for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013
////////////////////////////////////////////////////////////////////////////////

#include <ui/text/TestRunner.h>
#include <cppunit/TestRunner.h>
#include <wx/msgdlg.h>
#include "test.h"

wxIMPLEMENT_APP(wxExTestApp);

bool wxExTestApp::OnInit()
{
  SetAppName("wxex-test-gui");

  if (!wxExApp::OnInit())
  {
    return false;
  }

  m_Frame = new wxExTestFrame(NULL, 
    wxID_ANY, wxTheApp->GetAppDisplayName());
    
  m_Frame->Show(true);

  wxLog::SetActiveTarget(new wxLogStderr());
  
  return true;
}

int wxExTestApp::OnRun()
{
  wxExApp::OnRun();
  
  return !m_Frame->Success();
}

BEGIN_EVENT_TABLE(wxExTestFrame, wxExManagedFrame)
  EVT_TIMER(-1, wxExTestFrame::OnTimer)
END_EVENT_TABLE()

wxExTestFrame::wxExTestFrame(wxWindow* parent,
  wxWindowID id,
  const wxString& title,
  long style)
  : wxExManagedFrame(parent, id, title, style)
  , m_Timer(this)
  , m_Success(false)
{
  m_Timer.Start(10, true);
}

void wxExTestFrame::OnTimer(wxTimerEvent& event)
{
  CppUnit::TextUi::TestRunner runner;

  wxExAppTestSuite* suite = new wxExAppTestSuite;

  runner.addTest(suite);
  m_Success = runner.run("", false);
  
  wxTheApp->ExitMainLoop();
}
