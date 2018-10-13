////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/vcs.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include "test.h"

TEST_CASE("wex::vcs")
{
  // GetCount
  REQUIRE( wex::vcs::GetCount() > 0);

  wex::path file(GetTestPath("test.h"));
  file.MakeAbsolute();
  
  // In wex::app the vcs is Read, so current vcs is known,
  // using this constructor results in command id 1, being add.
  wex::vcs vcs(std::vector< wex::path >{file.Path().string()}, 1);
  
  vcs.ConfigDialog(wex::window_data().Button(wxAPPLY | wxCANCEL));
  
  // DirExists
  REQUIRE( vcs.DirExists(file));
  
#ifndef __WXMSW__
#ifndef __WXOSX__
  // Execute
  REQUIRE( vcs.Execute());
  
  /// ShowDialog.  
  REQUIRE( vcs.ShowDialog(wex::window_data().Button(wxAPPLY | wxCANCEL)));
  
  /// Request.  
  REQUIRE( vcs.Request(wex::window_data().Button(wxAPPLY | wxCANCEL)));

  // GetEntry  
  REQUIRE( vcs.GetEntry().BuildMenu(100, new wex::menu("test")) > 0);
  REQUIRE( vcs.GetEntry().GetStdOut().empty());
  REQUIRE( vcs.GetEntry().GetCommand().GetCommand() == "add");
  
  // GetBranch
  REQUIRE( vcs.GetBranch() == "master");

  // GetName
  REQUIRE( vcs.GetName() == "Auto");
  REQUIRE(!vcs.GetEntry().GetCommand().IsOpen());

  // LoadDocument
  REQUIRE( wex::vcs::LoadDocument());
  
  // SetEntryFromBase
  wxConfigBase::Get()->Write(_("Base folder"), wxGetCwd());
  REQUIRE( vcs.SetEntryFromBase());
  
  // Use
  REQUIRE( vcs.Use());
#endif
#endif
}
