////////////////////////////////////////////////////////////////////////////////
// Name:      test-vcs.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <iostream>
#include <wex/factory/vcs.h>
#include <wex/test/test.h>

TEST_CASE("wex::factory::vcs")
{
  wex::path         file(wex::test::get_path("test.h"));
  wex::factory::vcs vcs;

  REQUIRE(!vcs.is_dir_excluded(file.data().parent_path()));
  REQUIRE(!vcs.is_file_excluded(file));
  REQUIRE(!vcs.setup_exclude(file));
  REQUIRE(!vcs.is_setup());
}

TEST_CASE("wex::factory::vcs_admin")
{
  SECTION("constructor-empty")
  {
    const wex::factory::vcs_admin vcsa("", wex::path());
    const auto&                   tl(vcsa.toplevel());

    REQUIRE(tl.string().empty());
    REQUIRE(!vcsa.exists());
    REQUIRE(!vcsa.is_toplevel());
  }

  SECTION("constructor-other")
  {
    const wex::factory::vcs_admin vcsa(".git", wex::path::current());
    const auto&                   tl(vcsa.toplevel());

    REQUIRE(!tl.string().empty());
    REQUIRE(!tl.string().contains("wxWidgets"));
    REQUIRE(!vcsa.exists());
    REQUIRE(vcsa.is_toplevel());
  }

  SECTION("constructor-subproject")
  {
    const wex::factory::vcs_admin vcsa(
      ".git",
      wex::path("../../external/wxWidgets/README.md"));
    const auto& tl(vcsa.toplevel());

    CAPTURE(tl.string());
    REQUIRE(!tl.string().contains("wxWidgets"));
    REQUIRE(!vcsa.exists());
    REQUIRE(vcsa.is_toplevel());
  }
}
