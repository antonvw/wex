////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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

TEST_SUITE_BEGIN("wex::vcs");

TEST_CASE("wex::vcs")
{
  // size
  REQUIRE( wex::vcs::size() > 0);

  wex::path file(wex::test::get_path("test.h"));
  file.make_absolute();
  
  // In wex::app the vcs is Read, so current vcs is known,
  // using this constructor results in command id 2, being add.
  wex::vcs vcs(std::vector< wex::path >{file.string()}, 2);
  
  vcs.config_dialog(wex::window_data().button(wxAPPLY | wxCANCEL));
  
  // dir_exists
  REQUIRE( vcs.dir_exists(file));
  
#ifndef __WXMSW__
  // Execute
  REQUIRE( vcs.execute());
  REQUIRE( vcs.execute("status"));
  REQUIRE(!vcs.execute("xxx"));
  
  /// show_dialog.  
  REQUIRE( vcs.show_dialog(wex::window_data().button(wxAPPLY | wxCANCEL)));
  
  /// request.  
  REQUIRE( vcs.request(wex::window_data().button(wxAPPLY | wxCANCEL)));

  // entry  
  REQUIRE( vcs.entry().build_menu(100, new wex::menu("test")) > 0);
  REQUIRE(!vcs.entry().get_stdout().empty());
  REQUIRE( vcs.entry().get_command().get_command() == "blame");
  
  // get_branch
  REQUIRE(!vcs.get_branch().empty());

  // name
  REQUIRE( vcs.name() == "Auto");
  REQUIRE( vcs.entry().get_command().is_open());

  // load_document
  REQUIRE( wex::vcs::load_document());
  
  // set_entry_from_base
  wex::config(_("Base folder")).set(std::list<std::string>{wxGetCwd().ToStdString()});
  REQUIRE( vcs.set_entry_from_base());
  
  // Use
  REQUIRE( vcs.use());
#endif
}

TEST_SUITE_END();
