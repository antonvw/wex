////////////////////////////////////////////////////////////////////////////////
// Name:      test-shell.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/shell.h>
#include <wx/extension/managedframe.h>
#include "test.h"

void fixture::testShell()
{
  wxExSTCShell* shell = new wxExSTCShell(m_Frame);
  
  CPPUNIT_ASSERT(shell->GetShellEnabled());
  
  shell->Prompt("test1");
  shell->Prompt("test2");
  shell->Prompt("test3");
  shell->Prompt("test4");

  // Prompting does not add a command to history.
  CPPUNIT_ASSERT(!shell->GetHistory().Contains("test4"));

  // Post 3 'a' chars to the shell, and check whether it comes in the history.
  Process("aaa\r", shell);
  CPPUNIT_ASSERT(shell->GetHistory().Contains("aaa"));
  CPPUNIT_ASSERT(shell->GetPrompt() == ">");
  CPPUNIT_ASSERT(shell->GetCommand() == "aaa");
  
  // Post 3 'b' chars to the shell, and check whether it comes in the history.
  Process("bbb\r", shell);
  CPPUNIT_ASSERT(shell->GetHistory().Contains("aaa"));
  CPPUNIT_ASSERT(shell->GetHistory().Contains("bbb"));
  CPPUNIT_ASSERT(shell->GetPrompt() == ">");
  CPPUNIT_ASSERT(shell->GetCommand() == "bbb");
  
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
  CPPUNIT_ASSERT(!shell->GetShellEnabled());
  
  CPPUNIT_ASSERT(!shell->SetPrompt("---------->"));
  CPPUNIT_ASSERT( shell->GetPrompt() == ">");
  
  CPPUNIT_ASSERT(!shell->Prompt("test1"));
  CPPUNIT_ASSERT(!shell->Prompt("test2"));
  CPPUNIT_ASSERT( shell->GetPrompt() == ">");
  
  shell->EnableShell(true);
  CPPUNIT_ASSERT( shell->GetShellEnabled());
  
  shell->Paste();
  
  // Test shell commands.
  shell->SetText("");
  shell->Undo(); // to reset command in shell
  Process("history\r", shell);
  CPPUNIT_ASSERT( shell->GetText().Contains("aaa"));
  CPPUNIT_ASSERT( shell->GetText().Contains("bbb"));
  
  shell->SetText("");
  Process("!1\r", shell);
  CPPUNIT_ASSERT( shell->GetText().Contains("aaa"));
  CPPUNIT_ASSERT(!shell->GetText().Contains("bbb"));
  
  shell->SetText("");
  Process("!a\r", shell);
  CPPUNIT_ASSERT( shell->GetText().Contains("aaa"));
  CPPUNIT_ASSERT(!shell->GetText().Contains("bbb"));
}
