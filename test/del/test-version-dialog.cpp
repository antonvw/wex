////////////////////////////////////////////////////////////////////////////////
// Name:      test-version-dialog.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/test/test.h>
#include <wex/del/version-dialog.h>

TEST_CASE("wex::about_info")
{
  wex::about_info about;

  about.icon(wxIcon());

  REQUIRE(about.GetDescription().empty());
  REQUIRE(about.GetDevelopers().empty());
  REQUIRE(about.GetLicence().empty());
  REQUIRE(about.GetWebSiteURL().empty());

  REQUIRE(about.description("xx").GetDescription() == "xx");
  REQUIRE(about.developer("yy").GetDevelopers()[0] == "yy");
  REQUIRE(about.licence("zz").GetLicence() == "zz");
  REQUIRE(about.website("www.xyz").GetWebSiteURL() == "www.xyz");
}

TEST_CASE("wex::extern_libraries")
{
  REQUIRE(wex::external_libraries().str().contains("wex"));
}

TEST_CASE("wex::version_info_dialog")
{
  SUBCASE("default-constructor")
  {
    const wex::version_info_dialog info;

    REQUIRE(info.about().GetDescription().find("wex") != std::string::npos);
  }

  SUBCASE("constructor")
  {
    wex::about_info about;
    about.description("hello");
    const wex::version_info_dialog info(about);

    REQUIRE(info.about().GetDescription().find("hello") != std::string::npos);
  }

  SUBCASE("constructor-other")
  {
    wex::version_info vi;

    wex::about_info about;
    about.description("hello");

    const wex::version_info_dialog info(vi, about);

    REQUIRE(info.about().GetDescription().find("hello") != std::string::npos);
  }
}
