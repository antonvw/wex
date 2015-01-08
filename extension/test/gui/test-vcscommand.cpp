////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcscommand.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcscommand.h>
#include "test.h"

void wxExGuiTestFixture::testVCSCommand()
{
  const wxExVCSCommand add("a&dd");
  const wxExVCSCommand blame("blame");
  const wxExVCSCommand co("checkou&t");
  const wxExVCSCommand commit("commit", "main");
  const wxExVCSCommand diff("diff", "popup", "submenu");
  const wxExVCSCommand log("log", "main");
  const wxExVCSCommand help("h&elp", "error", "", "m&e");
  const wxExVCSCommand update("update");
  const wxExVCSCommand none;

  CPPUNIT_ASSERT(add.GetCommand() == "add");
  CPPUNIT_ASSERT(add.GetCommand(true, true) == "a&dd");
  CPPUNIT_ASSERT(help.GetCommand() == "help me");
  CPPUNIT_ASSERT(help.GetCommand(true, true) == "h&elp m&e");
  CPPUNIT_ASSERT(help.GetCommand(false, true) == "h&elp");
  CPPUNIT_ASSERT(help.GetCommand(false, false) == "help");
  
  CPPUNIT_ASSERT(add.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);
  CPPUNIT_ASSERT(blame.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);
  CPPUNIT_ASSERT(commit.GetType() == wxExVCSCommand::VCS_COMMAND_IS_MAIN);
  CPPUNIT_ASSERT(diff.GetType() == wxExVCSCommand::VCS_COMMAND_IS_POPUP);
  CPPUNIT_ASSERT(help.GetType() == wxExVCSCommand::VCS_COMMAND_IS_BOTH);

  CPPUNIT_ASSERT(add.IsAdd());
  CPPUNIT_ASSERT(blame.IsBlame());
  CPPUNIT_ASSERT(co.IsCheckout());
  CPPUNIT_ASSERT(commit.IsCommit());
  CPPUNIT_ASSERT(diff.IsDiff());
  CPPUNIT_ASSERT(help.IsHelp());
  CPPUNIT_ASSERT(log.IsHistory());
  CPPUNIT_ASSERT(blame.IsOpen());
  CPPUNIT_ASSERT(update.IsUpdate());
  CPPUNIT_ASSERT(!help.UseFlags());
  CPPUNIT_ASSERT(help.UseSubcommand());

  CPPUNIT_ASSERT(add.GetSubMenu().empty());
  CPPUNIT_ASSERT(diff.GetSubMenu() == "submenu");
  CPPUNIT_ASSERT(help.GetSubMenu() == "m&e");

  CPPUNIT_ASSERT(none.GetType() == wxExVCSCommand::VCS_COMMAND_IS_NONE);
}
