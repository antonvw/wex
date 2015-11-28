////////////////////////////////////////////////////////////////////////////////
// Name:      test-process->cpp
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
  
  wxExProcess* process = new wxExProcess;
  
  CPPUNIT_ASSERT(!process->GetError());
  CPPUNIT_ASSERT( process->GetOutput().empty());
  CPPUNIT_ASSERT(!process->HasStdError());
  CPPUNIT_ASSERT(!process->IsRunning());
  if (process->GetShell() != nullptr)
    process->GetShell()->SetText(wxEmptyString);
  
  process->ConfigDialog(m_Frame, "test process", false);
  
  // Test wxEXEC_SYNC process->
  CPPUNIT_ASSERT( process->Execute("ls -l", wxEXEC_SYNC));
  CPPUNIT_ASSERT(!process->GetError());
  CPPUNIT_ASSERT(!process->GetOutput().empty());
  
  CPPUNIT_ASSERT(!process->IsRunning());
  CPPUNIT_ASSERT( process->IsSelected());
  CPPUNIT_ASSERT( process->GetShell() != nullptr);
  CPPUNIT_ASSERT( process->GetShell()->GetText().empty());
  CPPUNIT_ASSERT( process->Kill() == wxKILL_NO_PROCESS);
  
  process->ShowOutput();

  // Repeat last wxEXEC_SYNC process (using "" only for dialogs).
  CPPUNIT_ASSERT( process->Execute("ls -l", wxEXEC_SYNC));
  CPPUNIT_ASSERT(!process->GetError());
  CPPUNIT_ASSERT(!process->GetOutput().empty());

  // Test working directory for wxEXEC_SYNC process (should not change).
  CPPUNIT_ASSERT( process->Execute("ls -l", wxEXEC_SYNC, ".."));
  CPPUNIT_ASSERT(!process->GetError());
  CPPUNIT_ASSERT(!process->GetOutput().empty());
  CPPUNIT_ASSERT( wxGetCwd().Contains("data"));

  // Test invalid wxEXEC_SYNC process->
  CPPUNIT_ASSERT(!process->Execute("xxxx", wxEXEC_SYNC));
  
  // Test wxEXEC_ASYNC process->
  CPPUNIT_ASSERT( process->Execute("bash"));
  CPPUNIT_ASSERT( process->IsRunning());
  wxExShell* shell = process->GetShell();  
  CPPUNIT_ASSERT( shell != nullptr);
  Process("cd ~\rpwd\r", shell);
  CPPUNIT_ASSERT( shell->GetText().Contains("home"));
  CPPUNIT_ASSERT( cwd != wxGetCwd());
  CPPUNIT_ASSERT( process->Kill() == wxKILL_OK);

  // Test working directory for wxEXEC_ASYNC process (should change).
  CPPUNIT_ASSERT( process->Execute("ls -l", wxEXEC_ASYNC, ".."));
  CPPUNIT_ASSERT(!process->GetError());
  CPPUNIT_ASSERT(!wxGetCwd().Contains("data"));
  wxSetWorkingDirectory(cwd);
  CPPUNIT_ASSERT( process->Kill() == wxKILL_OK);

  // Test invalid wxEXEC_ASYNC process (the process gets a process id, and exits immediately).
  CPPUNIT_ASSERT( process->Execute("xxxx"));
  CPPUNIT_ASSERT(!process->GetError());
  // The output is not touched by the async process, so if it was not empty,
  // it still is not empty.
  CPPUNIT_ASSERT( process->GetOutput().empty());
  CPPUNIT_ASSERT( process->Kill() == wxKILL_OK);
  
  wxExProcess::PrepareOutput(m_Frame); // in fact already done

  // Go back to where we were, necessary for other tests.
  wxSetWorkingDirectory(cwd);
}
