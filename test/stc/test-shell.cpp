////////////////////////////////////////////////////////////////////////////////
// Name:      test-shell.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/shell.h>

#include "test.h"

/// Processes string on shell.
void process(const std::string& str, wex::shell* shell);

void process(const std::string& str, wex::shell* shell)
{
  for (unsigned i = 0; i < str.length(); ++i)
  {
    shell->process_char(str.at(i));
  }
}

TEST_CASE("wex::shell")
{
  auto* shell = new wex::shell();
  frame()->pane_add(shell);

  REQUIRE(shell->is_enabled());

  shell->prompt("test1");
  shell->prompt("test2");
  shell->prompt("test3");
  shell->prompt("test4");

  // Prompting does not add a command to history.
  REQUIRE(!shell->get_history().contains("test4"));

  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  process("aaa\r", shell);
  REQUIRE(shell->get_history().contains("aaa"));
  REQUIRE(shell->get_prompt() == ">");
  REQUIRE(shell->get_command() == "aaa");

  // Post 3 'b' chars to the shell, and check whether it comes in the history.
  process("bbb\r", shell);
  REQUIRE(shell->get_history().contains("aaa"));
  REQUIRE(shell->get_history().contains("bbb"));
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
  REQUIRE(shell->get_prompt() == ">");

  REQUIRE(!shell->prompt("test1"));
  REQUIRE(!shell->prompt("test2"));
  REQUIRE(shell->get_prompt() == ">");

  shell->enable(true);
  REQUIRE(shell->is_enabled());

  shell->Paste();

  // Test shell commands.
  shell->SetText("");
  shell->Undo(); // to reset command in shell
  process("history\r", shell);
  REQUIRE(shell->get_text().contains("aaa"));
  REQUIRE(shell->get_text().contains("bbb"));

  shell->SetText("");
  process("!1\r", shell);
  REQUIRE(shell->get_text().contains("aaa"));
  REQUIRE(!shell->get_text().contains("bbb"));

  shell->SetText("");
  process("!a\r", shell);
  REQUIRE(shell->get_text().contains("aaa"));
  REQUIRE(!shell->get_text().contains("bbb"));

  shell->set_process(nullptr);

  shell->DocumentEnd();

  wxKeyEvent event(wxEVT_KEY_DOWN);

  for (auto id :
       std::vector<int>{WXK_DOWN, WXK_UP, WXK_HOME, WXK_BACK, WXK_DELETE})
  {
    event.m_keyCode = id;
    wxPostEvent(shell, event);
  }
}
