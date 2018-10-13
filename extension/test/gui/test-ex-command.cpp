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
#include <wx/extension/ex-command.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wex::ex_command")
{
  wex::stc* stc = GetSTC();
  stc->SetText("more text\notherline\nother line");

  SUBCASE("Constructor STC")
  {
    wex::ex_command command(stc);

    REQUIRE( command.Command().empty());
    REQUIRE(!command.IsHandled());
    REQUIRE( command.STC() == stc);
    REQUIRE( command.Type() == wex::ex_command_type::NONE);

    command.Command("G");
    REQUIRE( command.Command() == "G");
    REQUIRE( command.Exec() );
    REQUIRE( stc->GetCurrentLine() == 2);

    command.clear();
    REQUIRE( command.Command().empty());
    command.Append('g');
    REQUIRE( command.Command() == "g");
    command.Append('g');
    REQUIRE( command.Command() == "gg");
    REQUIRE( command.front() == 'g');
    REQUIRE( command.back() == 'g');
    REQUIRE( command.size() == 2);
    command.pop_back();
    REQUIRE( command.size() == 1);
    REQUIRE( command.AppendExec('g'));
    REQUIRE( stc->GetCurrentLine() == 0);

    command.Set(wex::ex_command("dd"));
    REQUIRE( command.Command() == "dd");
    REQUIRE( command.STC() == stc);
    command.Restore(wex::ex_command("ww"));
    REQUIRE( command.Command() == "ww");
    REQUIRE( command.STC() == stc);
  }
  
  SUBCASE("Constructor command")
  {
    wex::ex_command command("G");

    REQUIRE( command.Command() == "G");
    REQUIRE(!command.IsHandled());
    REQUIRE( command.STC() == nullptr);
    REQUIRE( command.Type() == wex::ex_command_type::VI);

    REQUIRE(!command.Exec() );
    REQUIRE( stc->GetCurrentLine() == 0);
  }
}
