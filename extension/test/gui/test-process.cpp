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
#include <wex/process.h>
#include <wex/managedframe.h>
#include <wex/shell.h>
#include "test.h"

TEST_CASE("wex::process")
{
  // Test commands entered in shell.
  wex::path cwd;
  
  wex::process* process = new wex::process;
  
  REQUIRE(!process->error());
  REQUIRE( process->get_stdout().empty());
  REQUIRE( process->get_stderr().empty());
  REQUIRE(!process->is_running());
  process->get_shell()->SetText(std::string());
  
  process->config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL));
  
#ifdef __UNIX__
  // Test wait for prcess (sync)
#ifndef __WXOSX__
  REQUIRE( process->execute("ls -l", wex::process::EXEC_WAIT));
  REQUIRE(!process->error());
  REQUIRE(!process->write("hello world"));
  REQUIRE(!process->get_stdout().empty());
  
  REQUIRE(!process->is_running());
  REQUIRE(!process->get_command().empty());
  REQUIRE(!process->kill());
  
  process->show_output();

  // Repeat last process (using "" only for dialogs).
  REQUIRE( process->execute("ls -l", wex::process::EXEC_WAIT));
  REQUIRE(!process->error());
  REQUIRE(!process->get_stdout().empty());

  // Test working directory (should not change).
  REQUIRE( process->execute("ls -l", wex::process::EXEC_WAIT, ".."));
  REQUIRE(!process->error());
  REQUIRE(!process->get_stdout().empty());
  REQUIRE( wxGetCwd().Contains("data"));

  // Test invalid process
  REQUIRE(!process->execute("xxxx", wex::process::EXEC_WAIT));
  REQUIRE( process->get_stderr().empty());
  REQUIRE( process->get_stdout().empty());
  REQUIRE(!process->kill());
  
  // Test not wait for process (async)
  REQUIRE( process->execute("bash"));
  REQUIRE( process->is_running());
  wex::shell* shell = process->get_shell();  
  REQUIRE( shell != nullptr);
  Process("cd ~\rpwd\r", shell);
  REQUIRE( shell->GetText().Contains("home"));
  REQUIRE( cwd.original() != wex::path::Current());
  REQUIRE( process->kill());

  // Test working directory for process (should change).
  REQUIRE( process->execute("ls -l", wex::process::EXEC_DEFAULT, ".."));
  REQUIRE(!process->error());
  REQUIRE(!wxGetCwd().Contains("data"));
  wex::path::Current(cwd.original());
  REQUIRE( process->kill());
  
  // Test invalid process (the process gets a process id, and exits immediately).
  REQUIRE( process->execute("xxxx"));
  REQUIRE(!process->error());
  REQUIRE( process->kill());
#endif
#endif
  
  wex::process::prepare_output(frame()); // in fact already done

  // kill_all is done in main.
}
