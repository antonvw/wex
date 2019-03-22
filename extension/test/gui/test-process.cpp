////////////////////////////////////////////////////////////////////////////////
// Name:      test-process.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
  
  REQUIRE( process->get_stdout().empty());
  REQUIRE( process->get_stderr().empty());
  REQUIRE(!process->is_running());
  process->get_shell()->SetText(std::string());
  
  process->config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL));
  
#ifdef __UNIX__
  // Test wait for prcess (sync)
  REQUIRE( process->execute("ls -l", wex::process::EXEC_WAIT));
  REQUIRE(!process->write("hello world"));
  REQUIRE(!process->get_stdout().empty());
  
  REQUIRE(!process->is_running());
  REQUIRE(!process->get_command_executed().empty());
  REQUIRE(!process->kill());
  
  process->show_output();

  // Repeat last process (using "" only for dialogs).
  REQUIRE( process->execute("ls -l", wex::process::EXEC_WAIT));
  REQUIRE(!process->get_stdout().empty());

  // Test working directory (should not change).
  REQUIRE( process->execute("ls -l", wex::process::EXEC_WAIT, ".."));
  REQUIRE(!process->get_stdout().empty());
  REQUIRE( wxGetCwd().Contains("data"));

  // Test invalid process
  REQUIRE(!process->execute("xxxx", wex::process::EXEC_WAIT));
  REQUIRE(!process->get_stderr().empty());
  REQUIRE( process->get_stdout().empty());
  REQUIRE(!process->kill());
  
  // Test not wait for process (async)
  REQUIRE( process->execute("bash"));
  REQUIRE( process->is_running());
  wex::shell* shell = process->get_shell();  
  REQUIRE( shell != nullptr);
  ::process("cd ~\rpwd\r", shell);
  REQUIRE( shell->GetText().size() > 50);
  REQUIRE( cwd.original() != wex::path::current());
  REQUIRE( process->kill());

  // Test working directory for process (should change).
  REQUIRE( process->execute("ls -l", wex::process::EXEC_NO_WAIT, ".."));
  REQUIRE(!wxGetCwd().Contains("data"));
  wex::path::current(cwd.original());
  REQUIRE( process->kill());
  
  // Test invalid process (the process gets a process id, and exits immediately).
  REQUIRE( process->execute("xxxx"));
  REQUIRE( process->kill());
#endif
  
  wex::process::prepare_output(frame()); // in fact already done
}
