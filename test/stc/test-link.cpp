////////////////////////////////////////////////////////////////////////////////
// Name:      test-link.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/stc/link.h>

#include "test.h"

#ifdef __UNIX__
TEST_CASE("wex::link")
{
  auto*     stc = get_stc();
  wex::link lnk;

  SECTION("large-line")
  {
    auto*       vi = &stc->get_vi();
    std::string line("\"xxxxxx\" ");
    for (int i = 0; i < 500; i++)
    {
      line.append("a large line ");

      if (i == 250)
      {
        line.append("\"" + wex::path("test.sh").string() + "\" ");
      }
    }

    vi->mode().escape();
    stc->SetReadOnly(false);

    REQUIRE(vi->command(":a|" + line));
    REQUIRE(vi->command("/test"));
    REQUIRE(vi->command(" "));
    REQUIRE(stc->link_open());

    std::string* name = nullptr;
    REQUIRE(vi->command("/test"));
    REQUIRE(vi->command(" "));
    REQUIRE(stc->link_open(
      wex::stc::link_t().set(wex::stc::LINK_OPEN).set(wex::stc::LINK_OPEN_MIME),
      name));
    REQUIRE(name == nullptr);

    std::string name_ok;
    REQUIRE(stc->link_open(
      wex::stc::link_t().set(wex::stc::LINK_OPEN).set(wex::stc::LINK_OPEN_MIME),
      &name_ok));
    REQUIRE(name_ok == "test.sh");

    name_ok = std::string(50, 'c');
    REQUIRE(vi->command("gg"));
    REQUIRE(!stc->link_open(
      wex::stc::link_t().set(wex::stc::LINK_OPEN).set(wex::stc::LINK_OPEN_MIME),
      &name_ok));
    REQUIRE(name_ok == std::string(50, 'c'));
  }

  SECTION("mime")
  {
    wex::data::control data;
    data.line(wex::link::LINE_OPEN_MIME);
    stc->get_file().file_new(wex::path("test.html"));
    REQUIRE(lnk.get_path("www.wxwidgets.org", data).data().empty());
    REQUIRE(
      lnk.get_path("xxx.wxwidgets.org", data, stc).string() == "test.html");
    REQUIRE(lnk.get_path("xx", data, stc).data() == "test.html");
  }

  SECTION("pairs")
  {
    wex::config(_("stc.link.Pairs")).set(wex::config::strings_t{{"[\t]"}});
    wex::config(_("stc.link.Include directory"))
      .set(wex::config::strings_t{{"/usr/bin"}});
    lnk.config_get();

    REQUIRE(
      wex::config("stc.link.Pairs").get(wex::config::strings_t{{}}).size() ==
      1);

    wex::data::control data;
    REQUIRE(
      lnk.get_path("here is a pair [whoami] present", data, stc).data() ==
      "/usr/bin/whoami");
  }

  SECTION("source")
  {
    wex::config(_("stc.link.Include directory"))
      .set(wex::config::strings_t{{"/usr/bin"}});
    lnk.config_get();

    wex::data::control data;
    stc->get_file().file_new(wex::path("test.sh"));

    REQUIRE(
      lnk.get_path("source whoami", data, stc).data() == "/usr/bin/whoami");
  }
}
#endif
