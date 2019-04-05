////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-command.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/ex-command.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::ex_command")
{
  wex::stc* stc = get_stc();
  stc->set_text("more text\notherline\nother line");

  SUBCASE("Constructor STC")
  {
    wex::ex_command command(stc);

    REQUIRE( command.command().empty());
    REQUIRE(!command.is_handled());
    REQUIRE( command.get_stc() == stc);
    REQUIRE( command.type() == wex::ex_command::type_t::NONE);

    command.command("G");
    REQUIRE( command.command() == "G");
    REQUIRE( command.exec() );
    REQUIRE( stc->GetCurrentLine() == 2);

    command.clear();
    REQUIRE( command.command().empty());
    command.append('g');
    REQUIRE( command.command() == "g");
    command.append('g');
    REQUIRE( command.command() == "gg");
    REQUIRE( command.front() == 'g');
    REQUIRE( command.back() == 'g');
    REQUIRE( command.size() == 2);
    command.pop_back();
    REQUIRE( command.size() == 1);
    REQUIRE( command.append_exec('g'));
    REQUIRE( stc->GetCurrentLine() == 0);

    command.set(wex::ex_command("dd"));
    REQUIRE( command.command() == "dd");
    REQUIRE( command.get_stc() == stc);
    command.restore(wex::ex_command("ww"));
    REQUIRE( command.command() == "ww");
    REQUIRE( command.get_stc() == stc);
  }
  
  SUBCASE("Constructor command")
  {
    wex::ex_command command("G");

    REQUIRE( command.command() == "G");
    REQUIRE(!command.is_handled());
    REQUIRE( command.get_stc() == nullptr);
    REQUIRE( command.type() == wex::ex_command::type_t::VI);

    REQUIRE(!command.exec() );
    REQUIRE( stc->GetCurrentLine() == 0);
  }
}
