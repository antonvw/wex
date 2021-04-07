////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/address.h>
#include <wex/addressrange.h>
#include <wex/ex-stream.h>
#include <wex/ex.h>
#include <wex/file.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/substitute-data.h>

#include "test.h"

void create_file()
{
  const std::string text("test1\ntest2\ntest3\ntest4\n\n");
  std::fstream      ifs("ex-mode.txt", std::ios_base::out);
  REQUIRE(ifs.write(text.c_str(), text.size()));
}

wex::file open_file()
{
  create_file();

  wex::file ifs("ex-mode.txt", std::ios_base::in | std::ios_base::out);
  REQUIRE(ifs.is_open());
  return ifs;
}

void write_file(wex::ex_stream& exs, int lines)
{
  REQUIRE(exs.write());
  REQUIRE(exs.get_line_count_request() == lines);
  REQUIRE(!exs.is_modified());
}

TEST_CASE("wex::ex_stream")
{
  auto* stc = get_stc();
  auto* ex  = new wex::ex(stc, wex::ex::EX);

  SUBCASE("constructor")
  {
    wex::ex_stream exs(ex);
    REQUIRE(exs.get_current_line() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count_request() == LINE_COUNT_UNKNOWN);

    exs.goto_line(5);
    REQUIRE(exs.get_current_line() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count_request() == LINE_COUNT_UNKNOWN);
  }

  SUBCASE("erase")
  {
    wex::file      ifs(open_file());
    wex::ex_stream exs(ex);
    exs.stream(ifs);

    wex::addressrange ar(ex, "1,2");

    REQUIRE(exs.erase(ar));
    REQUIRE(exs.is_modified());
    REQUIRE(exs.get_line_count_request() == 3);

    write_file(exs, 3);
  }

  SUBCASE("find")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(ex);
    REQUIRE(!exs.find(std::string("one")));

    exs.stream(ifs);
    REQUIRE(exs.find(std::string("one")));
    REQUIRE(!exs.is_modified());
    REQUIRE(exs.get_current_line() == 3);

    REQUIRE(!exs.find(std::string("xxxone")));
    REQUIRE(exs.get_current_line() == 3);
  }

  SUBCASE("insert")
  {
    wex::file      ifs(open_file());
    wex::ex_stream exs(ex);
    exs.stream(ifs);

    REQUIRE(!exs.insert_text(wex::address(ex, 0), "TEXT_BEFORE"));

    REQUIRE(exs.insert_text(wex::address(ex, 1), "TEXT_BEFORE"));
    REQUIRE(exs.insert_text(
      wex::address(ex, 3),
      "TEXT_AFTER",
      wex::ex_stream::INSERT_AFTER));
    REQUIRE(exs.is_modified());
  }

  SUBCASE("join")
  {
    wex::file      ifs(open_file());
    wex::ex_stream exs(ex);
    ifs.open();
    exs.stream(ifs);

    wex::addressrange ar(ex, "%");
    REQUIRE(ar.get_begin().get_line() == 1);

    REQUIRE(exs.get_line_count_request() == 5);
    REQUIRE(exs.join(ar));
    REQUIRE(exs.is_modified());

    // write_file(exs, 4);
    // REQUIRE(exs.get_line_count_request() == 1);
  }

  SUBCASE("markers")
  {
    wex::file      ifs(open_file());
    wex::ex_stream exs(ex);
    exs.stream(ifs);

    REQUIRE(exs.marker_add('x', 4));
    REQUIRE(exs.marker_line('x') == 4);
    REQUIRE(!exs.marker_delete('y'));
    REQUIRE(exs.marker_delete('x'));
    REQUIRE(!exs.marker_delete('x'));
    REQUIRE(exs.marker_line('x') == -1);
  }

  SUBCASE("previous")
  {
    wex::file ifs("test.md", std::ios_base::in);
    wex::ex_stream exs(ex);
    exs.stream(ifs);
    exs.goto_line(10);

    REQUIRE(exs.find(std::string("markdown"), -1, false));
    REQUIRE(!exs.is_modified());
    REQUIRE(exs.get_current_line() == 1);
    REQUIRE(exs.find(std::string("one")));
    REQUIRE(exs.get_current_line() == 5); // ??
  }

  SUBCASE("request")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(ex);
    exs.stream(ifs);

    REQUIRE(exs.get_line_count_request() == 9);
    REQUIRE(exs.get_line_count() == 9);
    REQUIRE(exs.get_line_count_request() == 9);
  }

  SUBCASE("stream")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(ex);
    exs.stream(ifs);
    REQUIRE(stc->get_text() == "# markdown\n");
    REQUIRE(exs.get_current_line() == 0);

    exs.goto_line(3);
    REQUIRE(exs.get_current_line() == 3);

    exs.goto_line(4);
    REQUIRE(exs.get_current_line() == 4);

    exs.goto_line(3);
    REQUIRE(exs.get_current_line() == 3);
  }

  SUBCASE("substitute")
  {
    wex::file      ifs(open_file());
    wex::ex_stream exs(ex);
    exs.stream(ifs);
    wex::find_replace_data::get()->set_regex(false);

    wex::addressrange ar(ex, "%");

    REQUIRE(exs.substitute(ar, wex::data::substitute("test", "1234")));
    REQUIRE(exs.is_modified());
  }

  SUBCASE("write")
  {
    wex::file      ifs(open_file());
    wex::ex_stream exs(ex);
    exs.stream(ifs);

    wex::addressrange ar(ex, "%");
    REQUIRE(exs.write(ar, "tmp.txt"));
  }

  if (wex::file info("ex-mode.txt"); info.is_open())
  {
    if (const auto buffer(info.read()); buffer != nullptr)
    {
      wex::log::trace("ex-mode.txt") << "\n" << *buffer;
    }
  }

  remove("ex-mode.txt");
  remove("tmp.txt");

  stc->visual(true);
}
