////////////////////////////////////////////////////////////////////////////////
// Name:      test-process->cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/process.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include "test.h"

TEST_CASE("wxExProcess")
{
  // Test commands entered in shell.
  const wxString cwd = wxGetCwd();
  
#ifdef __UNIX__
#if wxCHECK_VERSION(3,1,0)
  SECTION("Async")
  {
    wxExProcess process;
    REQUIRE( process.Execute("pwd"));
    REQUIRE( process.GetOutput().empty());
  }
#endif
#endif
  
  wxExProcess* process = new wxExProcess;
  
  REQUIRE(!process->GetError());
  REQUIRE( process->GetOutput().empty());
  REQUIRE(!process->HasStdError());
  REQUIRE(!process->IsRunning());
  process->GetShell()->SetText(wxEmptyString);
  
  process->ConfigDialog(GetFrame(), "test process", false);
  
#ifdef __UNIX__
  // Test wxEXEC_SYNC process
  REQUIRE( process->Execute("ls -l", wxEXEC_SYNC));
  REQUIRE(!process->GetError());
  REQUIRE(!process->GetOutput().empty());
  
  REQUIRE(!process->IsRunning());
  REQUIRE( process->IsSelected());
  REQUIRE( process->Kill() == wxKILL_NO_PROCESS);
  
  process->ShowOutput();

  // Repeat last wxEXEC_SYNC process (using "" only for dialogs).
  REQUIRE( process->Execute("ls -l", wxEXEC_SYNC));
  REQUIRE(!process->GetError());
  REQUIRE(!process->GetOutput().empty());

  // Test working directory for wxEXEC_SYNC process (should not change).
  REQUIRE( process->Execute("ls -l", wxEXEC_SYNC, ".."));
  REQUIRE(!process->GetError());
  REQUIRE(!process->GetOutput().empty());
  REQUIRE( wxGetCwd().Contains("data"));

  // Test invalid wxEXEC_SYNC process
  REQUIRE(!process->Execute("xxxx", wxEXEC_SYNC));
  
  // Test wxEXEC_ASYNC process
  REQUIRE( process->Execute("bash"));
  REQUIRE( process->IsRunning());
  wxExShell* shell = process->GetShell();  
  REQUIRE( shell != nullptr);
  Process("cd ~\rpwd\r", shell);
  REQUIRE( shell->GetText().Contains("home"));
  REQUIRE( cwd != wxGetCwd());
  REQUIRE( process->Kill() == wxKILL_OK);

  // Test working directory for wxEXEC_ASYNC process (should change).
  REQUIRE( process->Execute("ls -l", wxEXEC_ASYNC, ".."));
  REQUIRE(!process->GetError());
  REQUIRE(!wxGetCwd().Contains("data"));
  wxSetWorkingDirectory(cwd);
  REQUIRE( process->Kill() == wxKILL_OK);
  
  // Test invalid wxEXEC_ASYNC process (the process gets a process id, and exits immediately).
  REQUIRE( process->Execute("xxxx"));
  REQUIRE(!process->GetError());
  // The output is not touched by the async process, so if it was not empty,
  // it still is not empty.
  REQUIRE( process->GetOutput().empty());
  REQUIRE( process->Kill() == wxKILL_OK);
#endif
  
  wxExProcess::PrepareOutput(GetFrame()); // in fact already done

  // Go back to where we were, necessary for other tests.
  wxSetWorkingDirectory(cwd);
}
