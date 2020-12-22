////////////////////////////////////////////////////////////////////////////////
// Name:      test-ex-stream.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/addressrange.h>
#include <wex/ex-stream.h>
#include <wex/frd.h>
#include <wex/stc.h>

//#define DEBUG 1

TEST_CASE("wex::ex_stream")
{
  auto* stc = get_stc();

  SUBCASE("constructor")
  {
    wex::ex_stream exs(stc);
    REQUIRE(exs.get_current_line() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count_request() == LINE_COUNT_UNKNOWN);

    exs.goto_line(5);
    REQUIRE(exs.get_current_line() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count() == LINE_COUNT_UNKNOWN);
    REQUIRE(exs.get_line_count_request() == LINE_COUNT_UNKNOWN);
  }

  SUBCASE("stream")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    exs.stream(ifs);
    REQUIRE(stc->get_text() == "# markdown");
    REQUIRE(exs.get_current_line() == 0);

    exs.goto_line(3);
    REQUIRE(exs.get_current_line() == 3);

    exs.goto_line(4);
    REQUIRE(exs.get_current_line() == 4);
  }

  SUBCASE("erase")
  {
    {
      const std::string text("test1\ntest2\ntest3\ntest4\n\n");
      std::fstream ifs("erase.txt", std::ios_base::out);
      REQUIRE(ifs.write(text.c_str(), text.size()));
    }
  
    {
      wex::file ifs("erase.txt", std::ios_base::in | std::ios_base::out);
      REQUIRE(ifs.is_open());

      wex::ex_stream exs(stc);
      exs.stream(ifs);
      
      wex::addressrange ar(&stc->get_ex(), "1,2");
      
      REQUIRE(exs.erase(ar));
      REQUIRE(exs.get_line_count_request() == 4);
    }

#ifdef DEBUG
    system("cat erase.txt");
#endif

    remove("erase.txt");
  }
  
  SUBCASE("find")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    REQUIRE(!exs.find(std::string("one")));
    
    exs.stream(ifs);
    REQUIRE(exs.find(std::string("one")));
    REQUIRE(exs.get_current_line() == 3);

    REQUIRE(!exs.find(std::string("xxxone")));
    REQUIRE(exs.get_current_line() == 3);
  }
  
  SUBCASE("insert")
  {
    {
      const std::string text("test1\ntest2\ntest3\ntest4\n\n");
      std::fstream ifs("insert.txt", std::ios_base::out);
      REQUIRE(ifs.write(text.c_str(), text.size()));
    }
  
    wex::file ifs("insert.txt", std::ios_base::out);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    exs.stream(ifs);
    
    REQUIRE(exs.insert_text(0, "TEXT_BEFORE"));
    REQUIRE(exs.insert_text(3, "TEXT_AFTER", wex::ex_stream::INSERT_AFTER));

#ifdef DEBUG
    system("cat insert.txt");
#endif
    
    remove("insert.txt");
  }
  
  SUBCASE("request")
  {
    wex::file ifs("test.md", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    exs.stream(ifs);

    REQUIRE(exs.get_line_count_request() == 9);
    REQUIRE(exs.get_line_count() == 9);
    REQUIRE(exs.get_line_count_request() == 9);
  }
  
  SUBCASE("substitute")
  {
    {
      const std::string text("test1 xx\ntest2 yy\ntest3 zz\nxx test4\n\n");
      std::fstream ifs("substitute.txt", std::ios_base::out);
      REQUIRE(ifs.write(text.c_str(), text.size()));
    }
  
    wex::file ifs("substitute.txt", std::ios_base::in);
    REQUIRE(ifs.is_open());

    wex::ex_stream exs(stc);
    exs.stream(ifs);
    wex::find_replace_data::get()->set_regex(false);
    
    wex::addressrange ar(&stc->get_ex(), "%");
    
    REQUIRE(exs.substitute(ar, "test", "1234"));

#ifdef DEBUG
    system("cat substitute.txt");
#endif
    
    remove("substitute.txt");
  }
}
