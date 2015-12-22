////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcsentry.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/vcsentry.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/defs.h>
#include "test.h"

TEST_CASE("wxExVCSEntry", "[vcs]")
{
  REQUIRE( wxExVCSEntry().GetCommands() == 1); // the empty command
  
  wxExVCSEntry test("my-vcs", "./",
    {wxExVCSCommand("one", "main"), wxExVCSCommand("two", "main")},
    wxExVCSEntry::VCS_FLAGS_LOCATION_POSTFIX);
  
  REQUIRE( test.GetCommands() == 2);
  REQUIRE(!test.GetCommand().GetCommand().empty());
  REQUIRE(!test.AdminDirIsTopLevel());
  REQUIRE( test.GetAdminDir() == "./");
  REQUIRE( test.GetFlags().empty());
  REQUIRE( test.GetName() == "my-vcs");
  REQUIRE( test.GetOutput().empty());
  
  REQUIRE( wxExVCSEntry().ShowDialog(
    GetFrame(),
    "vcs",
    false) == wxID_CANCEL);
  
  test.ShowOutput();
  
  wxMenu menu;
  REQUIRE( test.BuildMenu(0, &menu) == 0);
  
  // This should have no effect.  
  REQUIRE(!test.SetCommand(5));
  REQUIRE(!test.SetCommand(ID_EDIT_VCS_LOWEST));
  REQUIRE(!test.SetCommand(ID_VCS_LOWEST));
  
  REQUIRE( test.GetCommands() == 2);
  REQUIRE( test.GetFlags().empty());
  REQUIRE( test.GetName() == "my-vcs");
  REQUIRE( test.GetOutput().empty());
  REQUIRE(!test.Execute());
  
  wxExVCSEntry git("git");
  REQUIRE( git.Execute()); // executes just git, shows help
  git.ShowOutput();
  
  wxExVCSEntry* git_async = new wxExVCSEntry("git", wxEmptyString, {wxExVCSCommand("status")});
  REQUIRE( git_async->Execute(wxEmptyString, wxExLexer(), wxEXEC_ASYNC));
  git_async->ShowOutput();
}
