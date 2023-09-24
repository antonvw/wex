////////////////////////////////////////////////////////////////////////////////
// Name:      test-vi-motion.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/util.h>
#include <wex/ui/frd.h>

#include "../ex/test.h"
#include "test.h"

// See stc/test-vi.cpp for testing goto and vim

TEST_CASE("wex::vi-motion")
{
  auto* stc = get_stc();
  auto* vi  = new wex::vi(get_stc());

  SUBCASE("find")
  {
    stc->set_text("some text to find another find");
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->command("/find"));
    REQUIRE(stc->GetCurrentPos() == 17);
    REQUIRE(vi->mode().is_command());
    REQUIRE(vi->command("yb"));
    REQUIRE(vi->mode().is_command());
    REQUIRE(!vi->command("/xfind"));

    stc->DocumentStart();
    REQUIRE(vi->command("2/find"));
    REQUIRE(stc->GetCurrentPos() == 30);
  }

  // Test motion commands: navigate, yank, delete, and change.
  SUBCASE("motion")
  {
    stc->set_text("xxxxxxxxxx second\nxxxxxxxx\naaaaaaaaaa\n");
    wex::find_replace_data::get()->set_find_string("xx");

    for (auto& motion_command : vi->motion_commands())
    {
      for (auto c : motion_command.first)
      {
        stc->set_text("xxxxxxxxxx\nyyyyyyyyyy\n"
                      "zzzzzzzzzz\nftFT\n"
                      "{section}yyyy\n"
                      "{anothersection}{finalsection}");

        // test navigate
        std::string nc(1, c);

        if (c == 'f' || c == 't' || c == 'F' || c == 'T' || c == '\'')
        {
          nc += "f";
        }

        CAPTURE(motion_command.first);
        CAPTURE(nc);
        REQUIRE(vi->command(nc));

        // test navigate while in block mode
        change_mode(vi, "K", wex::vi_mode::state_t::VISUAL_BLOCK);
        REQUIRE(vi->command(nc));
        REQUIRE(vi->mode().is_visual());
        change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
        REQUIRE(vi->mode().is_command());

        // test yank
        std::string mc(
          c == 'f' || c == 't' || c == 'F' || c == 'T' || c == '\'' ? 3 : 2,
          'y');

        mc[0] = 'y';
        mc[1] = c;

        CAPTURE(mc);
        REQUIRE(vi->command(mc));
        REQUIRE(vi->last_command() == mc);
        REQUIRE(vi->mode().is_command());

        // test delete
        mc[0] = 'd';
        CAPTURE(mc);
        REQUIRE(vi->command(mc));
        REQUIRE(vi->last_command() == mc);
        REQUIRE(vi->mode().is_command());

        // test change
        mc[0] = 'c';
        CAPTURE(mc);
        REQUIRE(vi->command(mc));
        REQUIRE(vi->last_command() == mc);
        change_mode(vi, wex::esc(), wex::vi_mode::state_t::COMMAND);
        REQUIRE(vi->mode().is_command());
      }
    }
  }
}
