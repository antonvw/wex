////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExVCS")
{
  // GetCount
  REQUIRE( wxExVCS::GetCount() > 0);

  wxExPath file(GetTestPath("test.h"));
  file.MakeAbsolute();
  
  // In wxExApp the vcs is Read, so current vcs is known,
  // using this constructor results in command id 1, being add.
  wxExVCS vcs(std::vector< wxExPath >{file.Path().string()}, 1);
  
  vcs.ConfigDialog(wxExWindowData().Button(wxAPPLY | wxCANCEL));
  
  // DirExists
  REQUIRE( vcs.DirExists(file));
  
#ifndef __WXMSW__
  // Execute
  REQUIRE( vcs.Execute());
  
  /// ShowDialog.  
  REQUIRE( vcs.ShowDialog(wxExWindowData().Button(wxAPPLY | wxCANCEL)));
  
  /// Request.  
  REQUIRE( vcs.Request(wxExWindowData().Button(wxAPPLY | wxCANCEL)));

  // GetEntry  
  REQUIRE( vcs.GetEntry().BuildMenu(100, new wxExMenu("test")) > 0);
  REQUIRE( vcs.GetEntry().GetStdOut().empty());
  REQUIRE( vcs.GetEntry().GetCommand().GetCommand() == "add");
  
  // GetBranch
  REQUIRE( vcs.GetBranch() == "master");

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
#endif
}
