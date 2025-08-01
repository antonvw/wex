////////////////////////////////////////////////////////////////////////////////
// Name:      test-vim.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/vi/vi.h>

#include "../ex/test.h"

#include "../../src/ex/vi/vim.h"

// see also stc/test-vim, this module here tests vim internally

void test_no_vim_command(wex::vi* vi, std::string& cmd)
{
  const std::string org(cmd);
  size_t            parsed = 0;
  wex::vim          vim(vi, cmd);

  REQUIRE(!vim.is_vim());
  REQUIRE(!vim.is_motion());
  REQUIRE(!vim.is_other());
  REQUIRE(!vim.motion(0, parsed, nullptr));
  REQUIRE(parsed == 0);

  REQUIRE(cmd == org);
}

void test_vim_other_command(wex::vim& vim)
{
  REQUIRE(vim.is_vim());
  REQUIRE(!vim.is_motion());
  REQUIRE(vim.is_other());
}

TEST_CASE("wex::vim")
{
  auto*  stc    = get_stc();
  auto*  vi     = new wex::vi(get_stc());
  size_t parsed = 0;

  SECTION("illegal")
  {
    for (auto& cmd : std::vector<std::string>{"", "w", "12345"})
    {
      test_no_vim_command(vi, cmd);
    }
  }

  SECTION("motion")
  {
    std::string cmd("gu");
    wex::vim    vim(vi, cmd);

    REQUIRE(vim.is_vim());
    REQUIRE(vim.is_motion());
    REQUIRE(!vim.is_other());
    REQUIRE(vim.motion(0, parsed, vi->motion_commands().front().second));
    REQUIRE(parsed == 1);
    REQUIRE(cmd == "gu");

    cmd = "guw";
    vim.motion_prep();
    REQUIRE(cmd == "w");
    REQUIRE(vim.motion(0, parsed, vi->motion_commands().front().second));
    REQUIRE(parsed == 1);
    REQUIRE(cmd == "w");
  }

  SECTION("other")
  {
    std::string cmd("z");
    wex::vim    vim(vi, cmd);

    test_vim_other_command(vim);

    REQUIRE(!vim.other());
    REQUIRE(parsed == 0);
    REQUIRE(cmd == "z");

    cmd = "zc";

    test_vim_other_command(vim);

    REQUIRE(vim.other());
    REQUIRE(parsed == 0);
    REQUIRE(cmd.empty());
  }
}
