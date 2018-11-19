////////////////////////////////////////////////////////////////////////////////
// Name:      test-ctags.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/ctags.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::ctags")
{
  wex::stc_data data;

  SUBCASE("tags default")
  {
    wex::ex* ex = &get_stc()->get_vi();

    REQUIRE( wex::ctags(ex).find("wxExTestApp") );
    REQUIRE( wex::ctags(frame()).find("wxExTestApp") );
    REQUIRE( wex::ctags(ex).auto_complete("wxExTest") == "wxExTestApp");

    wex::ctags_entry current;
    wex::ctags_entry filter;
    REQUIRE( wex::ctags(ex).find("wxExTestApp", current, filter));
    REQUIRE( current.kind() == "c" );
  }

  SUBCASE("tags non-existing file")
  {
    data.ctags_filename("xxx");
    wex::stc* stc = new wex::stc(std::string("test"), data);
    AddPane(frame(), stc);
    wex::ex* ex = &stc->get_vi();

    REQUIRE(!wex::ctags(ex).find("wxExTestApp") );
  }
  
  SUBCASE("tags own file")
  {
    data.ctags_filename("test-ctags");
    wex::stc* stc = new wex::stc(std::string("test"), data);
    AddPane(frame(), stc);
    wex::ex* ex = &stc->get_vi();

    REQUIRE(!wex::ctags(ex).find("") );
    REQUIRE(!wex::ctags(ex).next() );
    REQUIRE(!wex::ctags(ex).previous() );
    REQUIRE(!wex::ctags(ex).find("xxxx") );
    REQUIRE( wex::ctags(ex).find("wxExTestApp") );
    REQUIRE(!wex::ctags(ex).next() );
    REQUIRE( wex::ctags(ex).separator() != ' ');
  }
}
