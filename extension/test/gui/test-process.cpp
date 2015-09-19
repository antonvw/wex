////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/process.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include "test.h"

void fixture::testProcess()
{
  // Test commands entered in shell.
  const wxString cwd = wxGetCwd();
  
  wxExProcess process;
  
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT( process.GetOutput().empty());
  CPPUNIT_ASSERT(!process.HasStdError());
  CPPUNIT_ASSERT(!process.IsRunning());
  if (process.GetSTC() != NULL)
    process.GetSTC()->SetText(wxEmptyString);
  
  process.ConfigDialog(m_Frame, "test process", false);
  
  // Test wxEXEC_SYNC process.
  CPPUNIT_ASSERT( process.Execute("ls -l", wxEXEC_SYNC));
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());
  
  CPPUNIT_ASSERT(!process.IsRunning());
  CPPUNIT_ASSERT( process.IsSelected());
  CPPUNIT_ASSERT( process.GetSTC() != NULL);
  CPPUNIT_ASSERT( process.GetSTC()->GetText().empty());
  CPPUNIT_ASSERT( process.Kill() == wxKILL_NO_PROCESS);
  
  process.ShowOutput();

  // Repeat last wxEXEC_SYNC process (using "" only for dialogs).
  // Currently dialog might be cancelled, so do not check return value.
  CPPUNIT_ASSERT( process.Execute("ls -l", wxEXEC_SYNC));
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());

  // Test working directory for wxEXEC_SYNC process (should not change).
  CPPUNIT_ASSERT( process.Execute("ls -l", wxEXEC_SYNC, ".."));
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!process.GetOutput().empty());
  CPPUNIT_ASSERT( wxGetCwd().Contains("data"));

  // Test invalid wxEXEC_SYNC process.
  CPPUNIT_ASSERT(!process.Execute("xxxx", wxEXEC_SYNC));
  
  // Test wxEXEC_ASYNC process.
  CPPUNIT_ASSERT( process.Execute("bash"));
  CPPUNIT_ASSERT( process.IsRunning());
  wxExSTCShell* shell = process.GetShell();  
  CPPUNIT_ASSERT( shell != NULL);
  Process("cd ~\rpwd\r", shell);
  CPPUNIT_ASSERT( shell->GetText().Contains("home"));
  CPPUNIT_ASSERT( cwd != wxGetCwd());

  // Test working directory for wxEXEC_ASYNC process (should change).
  CPPUNIT_ASSERT( process.Execute("ls -l", wxEXEC_ASYNC, ".."));
  CPPUNIT_ASSERT(!process.GetError());
  CPPUNIT_ASSERT(!wxGetCwd().Contains("data"));
  wxSetWorkingDirectory(cwd);

  // Test invalid wxEXEC_ASYNC process (the process gets a process id, and exits immediately).
  CPPUNIT_ASSERT( process.Execute("xxxx"));
  CPPUNIT_ASSERT(!process.GetError());
  // The output is not touched by the async process, so if it was not empty,
  // it still is not empty.
  CPPUNIT_ASSERT( process.GetOutput().empty());
  
  // Go back to where we were, necessary for other tests.
  wxSetWorkingDirectory(cwd);
}
