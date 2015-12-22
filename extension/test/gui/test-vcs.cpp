////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/menu.h>
#include <wx/extension/vcs.h>
#include <wx/extension/managedframe.h>
#include "test.h"

TEST_CASE("wxExVCS", "[vcs]")
{
  // GetCount
  REQUIRE( wxExVCS::GetCount() > 0);

  wxFileName file(GetTestFile());
  file.Normalize();
  
  // In wxExApp the vcs is Read, so current vcs is known,
  // using this constructor results in command id 0,
  // giving the first command of current vcs, being add.
  wxExVCS vcs(std::vector< wxString >{file.GetFullPath()});
  
  vcs.ConfigDialog(GetFrame(), "test vcs", false);
  
  // DirExists
  REQUIRE( vcs.DirExists(file));
  
  // Execute
  REQUIRE( vcs.Execute());

  // GetEntry  
  REQUIRE( vcs.GetEntry().BuildMenu(100, new wxMenu("test")) > 0);
  REQUIRE( vcs.GetEntry().GetOutput().empty());
  REQUIRE( vcs.GetEntry().GetCommand().GetCommand() == "add");
  
  // GetFileName
  REQUIRE( vcs.GetFileName().IsOk());
  
  // GetName
  REQUIRE( vcs.GetName() == "Auto");
  REQUIRE(!vcs.GetEntry().GetCommand().IsOpen());
  
  // LoadDocument
  REQUIRE( wxExVCS::LoadDocument());
  
  // SetEntryFromBase
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  REQUIRE( vcs.SetEntryFromBase());
  
  // Use
  REQUIRE( vcs.Use());
}
