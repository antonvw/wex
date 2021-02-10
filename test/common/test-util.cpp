////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "../test.h"
#include <boost/algorithm/string.hpp>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/ex.h>
#include <wex/managed-frame.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vcs-command.h>

TEST_CASE("wex::util" * doctest::may_fail())
{
  std::list<std::string> l{"x", "y", "z"};

  const std::string rect("012z45678901234567890\n"
                         "123y56789012345678901\n"
                         "234x67890123456789012\n"
                         "345a78901234567890123\n"
                         "456b89012345678901234\n");

  const std::string sorted("012a78908901234567890\n"
                           "123b89019012345678901\n"
                           "234x67890123456789012\n"
                           "345y56781234567890123\n"
                           "456z45672345678901234\n");

  SUBCASE("combobox_as")
  {
    auto* cb = new wxComboBox(frame(), wxID_ANY);
    wex::test::add_pane(frame(), cb);
    wex::combobox_as<const std::list<std::string>>(cb, l);
  }

  SUBCASE("combobox_from_list")
  {
    auto* cb = new wxComboBox(frame(), wxID_ANY);
    wex::test::add_pane(frame(), cb);

    wex::combobox_from_list(cb, l);
    REQUIRE(cb->GetCount() == 3);
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

#ifdef __UNIX__
  SUBCASE("make")
  {
    wex::path cwd; // as /usr/bin/git changes wd
    REQUIRE(wex::make(wex::path("xxx")) != -1);
    REQUIRE(wex::make(wex::path("make.tst")) != -1);
    REQUIRE(wex::make(wex::path("/usr/bin/git")) != -1);
  }
#endif

  SUBCASE("open_files")
  {
    wex::path::current(wex::test::get_path().string());

    get_stc()->SetFocus();
    get_stc()->DiscardEdits();

    REQUIRE(wex::open_files(frame(), std::vector<wex::path>()) == 0);
    REQUIRE(
      wex::open_files(
        frame(),
        std::vector<wex::path>{
          wex::test::get_path("test.h").data(),
          "test.cpp",
          "*xxxxxx*.cpp"}) == 2);
    REQUIRE(
      wex::open_files(
        frame(),
        std::vector<wex::path>{wex::test::get_path("test.h").data()}) == 1);
    REQUIRE(
      wex::open_files(
        frame(),
        std::vector<wex::path>{"../../data/wex-menus.xml"}) == 1);
  }

  SUBCASE("open_files_dialog") {}

#ifdef __UNIX__
  SUBCASE("shell_expansion")
  {
    std::string command("xxx `pwd` `pwd`");
    REQUIRE(wex::shell_expansion(command));
    REQUIRE(command.find("`") == std::string::npos);

    command = "no quotes";
    REQUIRE(wex::shell_expansion(command));
    REQUIRE(command == "no quotes");

    command = "illegal process `xyz`";
    REQUIRE(!wex::shell_expansion(command));
    REQUIRE(command == "illegal process `xyz`");
  }
#endif

  SUBCASE("sort_selection")
  {
    get_stc()->SelectNone();
    REQUIRE(!wex::sort_selection(get_stc()));
    get_stc()->set_text("aaaaa\nbbbbb\nccccc\n");
    get_stc()->SelectAll();
    wxMilliSleep(10);
    REQUIRE(wex::sort_selection(get_stc()));
    wxMilliSleep(10);
    REQUIRE(wex::sort_selection(get_stc(), 0, 3, 10));
    wxMilliSleep(10);
    REQUIRE(!wex::sort_selection(get_stc(), 0, 20, 10));
  }

  SUBCASE("sort_selection block")
  {
    get_stc()->get_vi().command("\x1b");
    get_stc()->SelectNone();
    get_stc()->set_text(rect);

    // make a block selection, invoke sort, and check result
    REQUIRE(get_stc()->get_vi().mode().is_command());
    REQUIRE(get_stc()->get_vi().command("3 "));
    REQUIRE(get_stc()->get_vi().command("K"));
    REQUIRE(get_stc()->get_vi().mode().is_visual());
    REQUIRE(get_stc()->get_vi().command("4j"));
    REQUIRE(get_stc()->get_vi().command("5l"));
    REQUIRE(get_stc()->get_vi().mode().is_visual());

    REQUIRE(wex::sort_selection(get_stc(), 0, 3, 5));
    REQUIRE(
      boost::algorithm::trim_copy(get_stc()->get_text()) ==
      boost::algorithm::trim_copy(sorted));
    REQUIRE(wex::sort_selection(
      get_stc(),
      wex::string_sort_t().set(wex::STRING_SORT_DESCENDING),
      3,
      5));
    REQUIRE(get_stc()->get_text() != sorted);
  }

  SUBCASE("vcs_command_stc")
  {
    wex::vcs_command command;
    wex::vcs_command_stc(command, wex::lexer(get_stc()), get_stc());
    wex::vcs_command_stc(command, wex::lexer("cpp"), get_stc());
    wex::vcs_command_stc(command, wex::lexer(), get_stc());
  }

  SUBCASE("vcs_execute")
  {
    // wex::vcs_execute(frame(), 0, std::vector< std::string > {}); // calls
    // dialog
  }

  SUBCASE("xml_error")
  {
    wex::path              fn("xml-err.xml");
    pugi::xml_parse_result pr;
    pr.status = pugi::xml_parse_status::status_ok;
    wex::xml_error(fn, &pr);
  }
}
