////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-command.cpp
// Purpose:   Implementation for wxExtension unit testing
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

TEST_CASE("wxExExCommand")
{
  wxExSTC* stc = GetSTC();
  stc->SetText("more text\notherline\nother line");

  SUBCASE("Constructor STC")
  {
    wxExExCommand command(stc);

    REQUIRE( command.Command().empty());
    REQUIRE(!command.IsHandled());
    REQUIRE( command.STC() == stc);
    REQUIRE( command.Type() == wxExExCommandType::NONE);

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

    command.Set(wxExExCommand("dd"));
    REQUIRE( command.Command() == "dd");
    REQUIRE( command.STC() == stc);
    command.Restore(wxExExCommand("ww"));
    REQUIRE( command.Command() == "ww");
    REQUIRE( command.STC() == stc);
  }
  
  SUBCASE("Constructor command")
  {
    wxExExCommand command("G");

    REQUIRE( command.Command() == "G");
    REQUIRE(!command.IsHandled());
    REQUIRE( command.STC() == nullptr);
    REQUIRE( command.Type() == wxExExCommandType::VI);

    REQUIRE(!command.Exec() );
    REQUIRE( stc->GetCurrentLine() == 0);
  }
}
