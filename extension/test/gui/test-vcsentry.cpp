////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs_entry.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcsentry.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include "test.h"

TEST_CASE("wex::vcs_entry")
{
  REQUIRE( wex::vcs_entry().GetCommands().size() == 1); // the empty command
  
  wex::vcs_entry test("my-vcs", "./",
    {wex::vcs_command("one", "main"), wex::vcs_command("two", "main")},
    wex::vcs_entry::VCS_FLAGS_LOCATION_POSTFIX);
  
  REQUIRE( test.GetCommands().size() == 2);
  REQUIRE(!test.GetCommand().GetCommand().empty());
  REQUIRE(!test.AdminDirIsTopLevel());
  REQUIRE( test.GetAdminDir() == "./");
  REQUIRE( test.GetBranch().empty());
  REQUIRE( test.GetFlags().empty());
  REQUIRE( test.GetName() == "my-vcs");
  REQUIRE( test.GetStdOut().empty());
  
  REQUIRE( wex::vcs_entry().GetFlagsLocation() == wex::vcs_entry::VCS_FLAGS_LOCATION_POSTFIX);
  
  test.ShowOutput();
  
  wex::menu menu;
  REQUIRE( test.BuildMenu(0, &menu) == 0);

#ifndef __WXMSW__
#ifndef __WXOSX__
  // This should have no effect.  
  REQUIRE(!test.SetCommand(5));
  REQUIRE(!test.SetCommand(ID_EDIT_VCS_LOWEST));
  REQUIRE(!test.SetCommand(ID_VCS_LOWEST));
  
  REQUIRE( test.GetCommands().size() == 2);
  REQUIRE( test.GetFlags().empty());
  REQUIRE( test.GetName() == "my-vcs");
  REQUIRE( test.GetStdOut().empty());
  REQUIRE(!test.Execute());
  
  wex::vcs_entry git("git");
  REQUIRE( git.Execute()); // executes just git, shows help
  REQUIRE( git.GetStdOut().find("usage: ") != std::string::npos);
  git.ShowOutput();

  wex::vcs_entry* git_async = new wex::vcsentry("git", std::string(), {wex::vcs_command("status")});
  REQUIRE( git_async->Execute(std::string(), wex::lexer(), PROCESS_EXEC_WAIT));
  git_async->ShowOutput();
#endif
#endif
}
