////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/log-none.h>
#include <wex/core/vcs-command.h>

#include "test.h"

#include <vector>

TEST_CASE("wex::util", "[!mayfail]")
{
  const wex::strings_t l{"x", "y", "z"};
  const wex::strings_t ls{"x:0", "y:0", "z:1"};

  SECTION("auto_complete_filename")
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

  SECTION("combobox_as")
  {
    auto* cb = new wxComboBox(get_frame(), wxID_ANY);
    wex::combobox_as<const wex::strings_t>(cb, l);
    REQUIRE(l.size() == cb->GetCount());
    CAPTURE(cb->GetValue());
    REQUIRE(cb->GetValue() == "x");
  }

  SECTION("combobox_from_list")
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
  SECTION("compare_file")
  {
    wex::config(_("list.Comparator")).set("diff");

    REQUIRE(wex::compare_file(
      wex::test::get_path("test.h"),
      wex::test::get_path("test.h")));
  }
#endif

  SECTION("listbox_as")
  {
    auto* lb = new wxListBox(get_frame(), wxID_ANY);
    wex::listbox_as<const wex::strings_t>(lb, ls);
    REQUIRE(l.size() == lb->GetCount());
    REQUIRE(lb->GetString(0) == "x");
    REQUIRE(lb->GetStringSelection() == "z");
  }

  SECTION("listbox_to_list")
  {
    auto* lb = new wxListBox(get_frame(), wxID_ANY);
    wex::listbox_as<const wex::strings_t>(lb, ls);
    REQUIRE(lb->GetStringSelection() == "z");
    const auto& lo(wex::listbox_to_list(lb));
    REQUIRE(lo.size() == lb->GetCount());
    CAPTURE(lo.back());
    REQUIRE(lo.front() == "x:0");
  }

  SECTION("open_files")
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

  SECTION("process_match")
  {
    const auto   p(wex::test::get_path("test.h"));
    wxEvtHandler e;

    process_match(p, &e);
  }

#ifdef __UNIX__
  SECTION("shell_expansion")
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

  SECTION("vcs_command_stc")
  {
    wex::vcs_command command;
    wex::vcs_command_stc(command, wex::lexer(get_stc()), get_stc());
    wex::vcs_command_stc(command, wex::lexer("cpp"), get_stc());
    wex::vcs_command_stc(command, wex::lexer(), get_stc());
  }

  SECTION("xml_error")
  {
    wex::path              fn("xml-err.xml");
    pugi::xml_parse_result pr;
    pr.status = pugi::xml_parse_status::status_ok;
    wex::xml_error(fn, &pr);
  }
}
