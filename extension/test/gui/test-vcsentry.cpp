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
#include <wex/vcsentry.h>
#include <wex/managedframe.h>
#include <wex/defs.h>
#include "test.h"

TEST_CASE("wex::vcs_entry")
{
  REQUIRE( wex::vcs_entry().get_commands().size() == 1); // the empty command
  
  wex::vcs_entry test("my-vcs", "./",
    {wex::vcs_command("one", "main"), wex::vcs_command("two", "main")},
    wex::vcs_entry::FLAGS_LOCATION_POSTFIX);
  
  REQUIRE( test.get_commands().size() == 2);
  REQUIRE(!test.get_command().get_command().empty());
  REQUIRE(!test.admin_dir_is_toplevel());
  REQUIRE( test.admin_dir() == "./");
  REQUIRE( test.get_branch().empty());
  REQUIRE( test.get_flags().empty());
  REQUIRE( test.name() == "my-vcs");
  REQUIRE( test.get_stdout().empty());
  
  REQUIRE( wex::vcs_entry().flags_location() == wex::vcs_entry::FLAGS_LOCATION_POSTFIX);
  
  test.show_output();
  
  wex::menu menu;
  REQUIRE( test.build_menu(0, &menu) == 0);

#ifndef __WXMSW__
#ifndef __WXOSX__
  // This should have no effect.  
  REQUIRE(!test.set_command(5));
  REQUIRE(!test.set_command(wex::ID_EDIT_VCS_LOWEST));
  REQUIRE(!test.set_command(wex::ID_VCS_LOWEST));
  
  REQUIRE( test.get_commands().size() == 2);
  REQUIRE( test.get_flags().empty());
  REQUIRE( test.name() == "my-vcs");
  REQUIRE( test.get_stdout().empty());
  REQUIRE(!test.execute());
  
  wex::vcs_entry git("git");
  REQUIRE( git.execute()); // executes just git, shows help
  REQUIRE( git.get_stdout().find("usage: ") != std::string::npos);
  git.show_output();

  wex::vcs_entry* git_async = new wex::vcs_entry("git", std::string(), {wex::vcs_command("status")});
  REQUIRE( git_async->execute(std::string(), wex::lexer(), wex::process::EXEC_WAIT));
  git_async->show_output();
#endif
#endif
}
