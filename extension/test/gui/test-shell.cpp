////////////////////////////////////////////////////////////////////////////////
// Name:      test-shell.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/shell.h>
#include <wex/managedframe.h>
#include "test.h"

TEST_CASE("wex::shell")
{
  wex::shell* shell = new wex::shell();
  wex::test::add_pane(frame(), shell);
  
  REQUIRE(shell->is_enabled());
  
  shell->prompt("test1");
  shell->prompt("test2");
  shell->prompt("test3");
  shell->prompt("test4");

  // Prompting does not add a command to history.
  REQUIRE( shell->get_history().find("test4") == std::string::npos);

  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  process("aaa\r", shell);
  REQUIRE(shell->get_history().find("aaa") != std::string::npos);
  REQUIRE(shell->get_prompt() == ">");
  REQUIRE(shell->get_command() == "aaa");
  
  // Post 3 'b' chars to the shell, and check whether it comes in the history.
  process("bbb\r", shell);
  REQUIRE(shell->get_history().find("aaa") != std::string::npos);
  REQUIRE(shell->get_history().find("bbb") != std::string::npos);
  REQUIRE(shell->get_prompt() == ">");
  REQUIRE(shell->get_command() == "bbb");
  
  process("b\t", shell); // tests Expand
  shell->process_char(WXK_BACK);
  shell->process_char(WXK_BACK);
  shell->process_char(WXK_BACK);
  shell->process_char(WXK_BACK);
  shell->process_char(WXK_BACK);
  shell->process_char(WXK_DELETE);
  
  shell->DocumentEnd();
  
  shell->AppendText("hello");
  
  // Test shell enable/disable.
  shell->enable(false);
  REQUIRE(!shell->is_enabled());
  
  REQUIRE(!shell->set_prompt("---------->"));
  REQUIRE( shell->get_prompt() == ">");
  
  REQUIRE(!shell->prompt("test1"));
  REQUIRE(!shell->prompt("test2"));
  REQUIRE( shell->get_prompt() == ">");
  
  shell->enable(true);
  REQUIRE( shell->is_enabled());
  
  shell->Paste();
  
  // Test shell commands.
  shell->SetText("");
  shell->Undo(); // to reset command in shell
  process("history\r", shell);
  REQUIRE( shell->GetText().find("aaa") != std::string::npos);
  REQUIRE( shell->GetText().find("bbb") != std::string::npos);
  
  shell->SetText("");
  process("!1\r", shell);
  REQUIRE( shell->GetText().find("aaa") != std::string::npos);
  REQUIRE( shell->GetText().find("bbb") == std::string::npos);
  
  shell->SetText("");
  process("!a\r", shell);
  REQUIRE( shell->GetText().find("aaa") != std::string::npos);
  REQUIRE( shell->GetText().find("bbb") == std::string::npos);
  
  shell->set_process(nullptr);
  
  shell->DocumentEnd();
  
  wxKeyEvent event(wxEVT_KEY_DOWN);
  
  for (auto id : std::vector<int> {
    WXK_DOWN, WXK_UP, WXK_HOME, WXK_BACK, WXK_DELETE}) 
  {
    event.m_keyCode = id;
    wxPostEvent(shell, event);
  }
}
