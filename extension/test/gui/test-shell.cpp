////////////////////////////////////////////////////////////////////////////////
// Name:      test-shell.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/shell.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExShell")
{
  wxExShell* shell = new wxExShell();
  AddPane(GetFrame(), shell);
  
  REQUIRE(shell->GetShellEnabled());
  
  shell->Prompt("test1");
  shell->Prompt("test2");
  shell->Prompt("test3");
  shell->Prompt("test4");

  // Prompting does not add a command to history.
  REQUIRE(!shell->GetHistory().find("test4") != std::string::npos);

  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  Process("aaa\r", shell);
  REQUIRE(shell->GetHistory().find("aaa") != std::string::npos);
  REQUIRE(shell->GetPrompt() == ">");
  REQUIRE(shell->GetCommand() == "aaa");
  
  // Post 3 'b' chars to the shell, and check whether it comes in the history.
  Process("bbb\r", shell);
  REQUIRE(shell->GetHistory().find("aaa") != std::string::npos);
  REQUIRE(shell->GetHistory().find("bbb") != std::string::npos);
  REQUIRE(shell->GetPrompt() == ">");
  REQUIRE(shell->GetCommand() == "bbb");
  
  Process("b\t", shell); // tests Expand
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_BACK);
  shell->ProcessChar(WXK_DELETE);
  
  shell->DocumentEnd();
  
  shell->AppendText("hello");
  
  // Test shell enable/disable.
  shell->EnableShell(false);
  REQUIRE(!shell->GetShellEnabled());
  
  REQUIRE(!shell->SetPrompt("---------->"));
  REQUIRE( shell->GetPrompt() == ">");
  
  REQUIRE(!shell->Prompt("test1"));
  REQUIRE(!shell->Prompt("test2"));
  REQUIRE( shell->GetPrompt() == ">");
  
  shell->EnableShell(true);
  REQUIRE( shell->GetShellEnabled());
  
  shell->Paste();
  
  // Test shell commands.
  shell->SetText("");
  shell->Undo(); // to reset command in shell
  Process("history\r", shell);
  REQUIRE( shell->GetText().find("aaa") != std::string::npos);
  REQUIRE( shell->GetText().find("bbb") != std::string::npos);
  
  shell->SetText("");
  Process("!1\r", shell);
  REQUIRE( shell->GetText().find("aaa") != std::string::npos);
  REQUIRE( shell->GetText().find("bbb") == std::string::npos);
  
  shell->SetText("");
  Process("!a\r", shell);
  REQUIRE( shell->GetText().find("aaa") != std::string::npos);
  REQUIRE( shell->GetText().find("bbb") == std::string::npos);
  
  shell->SetProcess(nullptr);
  
  shell->DocumentEnd();
  
  wxKeyEvent event(wxEVT_KEY_DOWN);
  
  for (auto id : std::vector<int> {
    WXK_DOWN, WXK_UP, WXK_HOME, WXK_BACK, WXK_DELETE}) 
  {
    event.m_keyCode = id;
    wxPostEvent(shell, event);
  }
}
