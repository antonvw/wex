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
#include <wex/vcs.h>
#include <wex/config.h>
#include <wex/managedframe.h>
#include <wex/menu.h>
#include "test.h"

TEST_CASE("wex::vcs")
{
  // GetCount
  REQUIRE( wex::vcs::GetCount() > 0);

  wex::path file(GetTestPath("test.h"));
  file.make_absolute();
  
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
  wex::config(_("Base folder")).set(wxGetCwd().ToStdString());
  REQUIRE( vcs.SetEntryFromBase());
  
  // Use
  REQUIRE( vcs.Use());
#endif
#endif
}
