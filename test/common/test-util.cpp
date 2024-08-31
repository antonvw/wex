////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/core/vcs-command.h>

#include "test.h"

#include <vector>

TEST_CASE("wex::util" * doctest::may_fail())
{
  const wex::strings_t l{"x", "y", "z"};

  SUBCASE("auto_complete_filename")
  {
    REQUIRE(wex::auto_complete_filename("te"));
    REQUIRE(wex::auto_complete_filename("te")->expansion == "st");
    REQUIRE(!wex::auto_complete_filename("XX"));

#ifdef __UNIX__
    REQUIRE(wex::auto_complete_filename("/usr/local/l"));

    // we are in wex/test/data
    REQUIRE(wex::auto_complete_filename("../../src/v"));
#endif
  }

  SUBCASE("combobox_as")
  {
    auto* cb = new wxComboBox(get_frame(), wxID_ANY);
    wex::combobox_as<const wex::strings_t>(cb, l);
  }

  SUBCASE("combobox_from_list")
  {
    auto* cb = new wxComboBox(get_frame(), wxID_ANY);

    wex::combobox_from_list(cb, l);
    REQUIRE(cb->GetCount() == 3);
    REQUIRE(cb->GetValue() == "x");

    wex::combobox_from_list(cb, wex::strings_t{"", "xx", "yy"});
    REQUIRE(cb->GetCount() == 3);
    REQUIRE(cb->GetValue().empty());

    wex::combobox_from_list(cb, wex::strings_t{});
    REQUIRE(cb->GetCount() == 0);
    REQUIRE(cb->GetValue().empty());
  }

#ifndef __WXMSW__
  SUBCASE("compare_file")
  {
    wex::config(_("list.Comparator")).set("diff");

    REQUIRE(wex::compare_file(
      wex::test::get_path("test.h"),
      wex::test::get_path("test.h")));
  }
#endif

  SUBCASE("open_files")
  {
    wex::path::current(wex::test::get_path().data());

    get_stc()->SetFocus();
    get_stc()->DiscardEdits();

    wex::log_none off;

    REQUIRE(wex::open_files(get_frame(), std::vector<wex::path>()) == 0);
    REQUIRE(
      wex::open_files(
        get_frame(),
        std::vector<wex::path>{wex::test::get_path()}) == 0); // dir
    REQUIRE(
      wex::open_files(
        get_frame(),
        std::vector<wex::path>{
          wex::test::get_path("test.h").data(),
          wex::path("test.cpp"),
          wex::path("*xxxxxx*.cpp")}) == 2);
    REQUIRE(
      wex::open_files(
        get_frame(),
        std::vector<wex::path>{wex::test::get_path("test.h")}) == 1);
    REQUIRE(
      wex::open_files(
        get_frame(),
        std::vector<wex::path>{wex::path("../../data/wex-menus.xml")}) == 1);
    REQUIRE(
      wex::open_files(
        get_frame(),
        std::vector<wex::path>{wex::path("../../data-xxx/yy.cpp")}) == 0);
  }

  SUBCASE("process_match")
  {
    const auto   p(wex::test::get_path("test.h"));
    wxEvtHandler e;

    process_match(p, &e);
  }

#ifdef __UNIX__
  SUBCASE("shell_expansion")
  {
    std::string command("xxx `pwd` `pwd`");
    REQUIRE(wex::shell_expansion(command));
    REQUIRE(!command.contains("`"));

    command = "no quotes";
    REQUIRE(wex::shell_expansion(command));
    REQUIRE(command == "no quotes");

    command = "echo 'xyz'";
    REQUIRE(wex::shell_expansion(command));
    REQUIRE(command == "echo 'xyz'");
  }
#endif

  SUBCASE("vcs_command_stc")
  {
    wex::vcs_command command;
    wex::vcs_command_stc(command, wex::lexer(get_stc()), get_stc());
    wex::vcs_command_stc(command, wex::lexer("cpp"), get_stc());
    wex::vcs_command_stc(command, wex::lexer(), get_stc());
  }

  SUBCASE("xml_error")
  {
    wex::path              fn("xml-err.xml");
    pugi::xml_parse_result pr;
    pr.status = pugi::xml_parse_status::status_ok;
    wex::xml_error(fn, &pr);
  }
}
