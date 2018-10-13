////////////////////////////////////////////////////////////////////////////////
// Name:      test-process->cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/process.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/shell.h>
#include "test.h"

TEST_CASE("wex::process")
{
  // Test commands entered in shell.
  wex::path cwd;
  
  wex::process* process = new wex::process;
  
  REQUIRE(!process->GetError());
  REQUIRE( process->GetStdOut().empty());
  REQUIRE( process->GetStdErr().empty());
  REQUIRE(!process->IsRunning());
  process->GetShell()->SetText(std::string());
  
  process->ConfigDialog(wex::window_data().Button(wxAPPLY | wxCANCEL));
  
#ifdef __UNIX__
  // Test wait for prcess (sync)
#ifndef __WXOSX__
  REQUIRE( process->Execute("ls -l", PROCESS_EXEC_WAIT));
  REQUIRE(!process->GetError());
  REQUIRE(!process->Write("hello world"));
  REQUIRE(!process->GetStdOut().empty());
  
  REQUIRE(!process->IsRunning());
  REQUIRE(!process->GetExecuteCommand().empty());
  REQUIRE(!process->Kill());
  
  process->ShowOutput();

  // Repeat last process (using "" only for dialogs).
  REQUIRE( process->Execute("ls -l", PROCESS_EXEC_WAIT));
  REQUIRE(!process->GetError());
  REQUIRE(!process->GetStdOut().empty());

  // Test working directory (should not change).
  REQUIRE( process->Execute("ls -l", PROCESS_EXEC_WAIT, ".."));
  REQUIRE(!process->GetError());
  REQUIRE(!process->GetStdOut().empty());
  REQUIRE( wxGetCwd().Contains("data"));

  // Test invalid process
  REQUIRE(!process->Execute("xxxx", PROCESS_EXEC_WAIT));
  REQUIRE( process->GetStdErr().empty());
  REQUIRE( process->GetStdOut().empty());
  REQUIRE(!process->Kill());
  
  // Test not wait for process (async)
  REQUIRE( process->Execute("bash"));
  REQUIRE( process->IsRunning());
  wex::shell* shell = process->GetShell();  
  REQUIRE( shell != nullptr);
  Process("cd ~\rpwd\r", shell);
  REQUIRE( shell->GetText().Contains("home"));
  REQUIRE( cwd.GetOriginal() != wex::path::Current());
  REQUIRE( process->Kill());

  // Test working directory for process (should change).
  REQUIRE( process->Execute("ls -l", PROCESS_EXEC_DEFAULT, ".."));
  REQUIRE(!process->GetError());
  REQUIRE(!wxGetCwd().Contains("data"));
  wex::path::Current(cwd.GetOriginal());
  REQUIRE( process->Kill());
  
  // Test invalid process (the process gets a process id, and exits immediately).
  REQUIRE( process->Execute("xxxx"));
  REQUIRE(!process->GetError());
  REQUIRE( process->Kill());
#endif
#endif
  
  wex::process::PrepareOutput(GetFrame()); // in fact already done

  // KillAll is done in main.
}
