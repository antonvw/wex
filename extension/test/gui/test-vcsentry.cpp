////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcsentry.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
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

void fixture::testVCSEntry()
{
  CPPUNIT_ASSERT( wxExVCSEntry().GetCommands() == 1); // the empty command
  
  wxExVCSEntry test("my-vcs", "./",
    {wxExVCSCommand("one", "main"), wxExVCSCommand("two", "main")},
    wxExVCSEntry::VCS_FLAGS_LOCATION_POSTFIX);
  
  CPPUNIT_ASSERT( test.GetCommands() == 2);
  CPPUNIT_ASSERT(!test.GetCommand().GetCommand().empty());
  CPPUNIT_ASSERT(!test.AdminDirIsTopLevel());
  CPPUNIT_ASSERT( test.GetAdminDir() == "./");
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName() == "my-vcs");
  CPPUNIT_ASSERT( test.GetOutput().empty());
  
  CPPUNIT_ASSERT( wxExVCSEntry().ShowDialog(
    m_Frame,
    "vcs",
    false) == wxID_CANCEL);
  
  test.ShowOutput();
  
  wxMenu menu;
  CPPUNIT_ASSERT( test.BuildMenu(0, &menu) == 0);
  
  // This should have no effect.  
  CPPUNIT_ASSERT(!test.SetCommand(5));
  CPPUNIT_ASSERT(!test.SetCommand(ID_EDIT_VCS_LOWEST));
  CPPUNIT_ASSERT(!test.SetCommand(ID_VCS_LOWEST));
  
  CPPUNIT_ASSERT( test.GetCommands() == 2);
  CPPUNIT_ASSERT( test.GetFlags().empty());
  CPPUNIT_ASSERT( test.GetName() == "my-vcs");
  CPPUNIT_ASSERT( test.GetOutput().empty());
  CPPUNIT_ASSERT(!test.Execute());
  
  wxExVCSEntry git("git");
  CPPUNIT_ASSERT( git.Execute()); // executes just git, shows help
  git.ShowOutput();
  
  wxExVCSEntry* git_async = new wxExVCSEntry("git", wxEmptyString, {wxExVCSCommand("status")});
  CPPUNIT_ASSERT( git_async->Execute(wxEmptyString, wxExLexer(), wxEXEC_ASYNC));
  git_async->ShowOutput();
}
