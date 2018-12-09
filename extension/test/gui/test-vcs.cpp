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
  // size
  REQUIRE( wex::vcs::size() > 0);

  wex::path file(get_testpath("test.h"));
  file.make_absolute();
  
  // In wex::app the vcs is Read, so current vcs is known,
  // using this constructor results in command id 1, being add.
  wex::vcs vcs(std::vector< wex::path >{file.data().string()}, 1);
  
  vcs.config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL));
  
  // dir_exists
  REQUIRE( vcs.dir_exists(file));
  
#ifndef __WXMSW__
#ifndef __WXOSX__
  // Execute
  REQUIRE( vcs.execute());
  
  /// show_dialog.  
  REQUIRE( vcs.show_dialog(wex::window_data().button(wxAPPLY | wxCANCEL)));
  
  /// request.  
  REQUIRE( vcs.request(wex::window_data().button(wxAPPLY | wxCANCEL)));

  // entry  
  REQUIRE( vcs.entry().build_menu(100, new wex::menu("test")) > 0);
  REQUIRE( vcs.entry().get_stdout().empty());
  REQUIRE( vcs.entry().get_command().get_command() == "add");
  
  // get_branch
  REQUIRE( vcs.get_branch() == "master");

  // name
  REQUIRE( vcs.name() == "Auto");
  REQUIRE(!vcs.entry().get_command().is_open());

  // load_document
  REQUIRE( wex::vcs::load_document());
  
  // set_entry_from_base
  wex::config(_("Base folder")).set(wxGetCwd().ToStdString());
  REQUIRE( vcs.set_entry_from_base());
  
  // Use
  REQUIRE( vcs.use());
#endif
#endif
}
