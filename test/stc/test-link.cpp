////////////////////////////////////////////////////////////////////////////////
// Name:      test-link.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/link.h>

#include "test.h"

#ifdef __UNIX__
TEST_CASE("wex::link")
{
  auto*     stc = get_stc();
  wex::link lnk;

  SUBCASE("mime")
  {
    wex::data::control data;
    data.line(wex::link::LINE_OPEN_MIME);
    stc->get_file().file_new(wex::path("test.html"));
    REQUIRE(lnk.get_path("www.wxwidgets.org", data).data().empty());
    REQUIRE(
      lnk.get_path("xxx.wxwidgets.org", data, stc).string() == "test.html");
    REQUIRE(lnk.get_path("xx", data, stc).data() == "test.html");
  }

  SUBCASE("pairs")
  {
    wex::config(_("stc.link.Pairs")).set(std::list<std::string>{{"[\t]"}});
    wex::config(_("stc.link.Include directory"))
      .set(std::list<std::string>{{"/usr/bin"}});
    lnk.config_get();

    wex::data::control data;
    REQUIRE(
      lnk.get_path("here is a pair [whoami] present", data, stc).data() ==
      "/usr/bin/whoami");
  }

  SUBCASE("source")
  {
    wex::config(_("stc.link.Include directory"))
      .set(std::list<std::string>{{"/usr/bin"}});
    lnk.config_get();

    wex::data::control data;
    stc->get_file().file_new(wex::path("test.sh"));

    REQUIRE(
      lnk.get_path("source whoami", data, stc).data() == "/usr/bin/whoami");
  }
}
#endif
