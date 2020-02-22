////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/util.h>
#include <wex/config.h>
#include <wex/ex.h>
#include <wex/lexers.h>
#include <wex/macros.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/vcscommand.h>
#include "test.h"

TEST_CASE("wex" * doctest::may_fail())
{
  std::list < std::string > l{"x","y","z"};
  std::vector<int> cs{'(',')','{','<','>'};
    
  const std::string rect("\
012z45678901234567890\n\
123y56789012345678901\n\
234x67890123456789012\n\
345a78901234567890123\n\
456b89012345678901234\n");

  const std::string sorted("\
012a78908901234567890\n\
123b89019012345678901\n\
234x67890123456789012\n\
345y56781234567890123\n\
456z45672345678901234\n");

  SUBCASE("after")
  {
    REQUIRE( wex::after("nospace", ' ', false) == "nospace");
    REQUIRE( wex::after("nospace", ' ', true) == "nospace");
    REQUIRE( wex::after("some space and more", ' ', false) == "more");
    REQUIRE( wex::after("some space and more", ' ', true) == "space and more");
    REQUIRE( wex::after("some space and more", 'm', false) == "ore");
  }

  SUBCASE("align_text")
  {
    REQUIRE( wex::align_text("test", "header", true, true,
      wex::lexers::get()->find("cpp")).size() 
        == std::string("// headertest").size());
  }
      
  SUBCASE("autocomplete_text")
  {
    REQUIRE( wex::ex::get_macros().load_document());
    std::string s;
    REQUIRE(!wex::autocomplete_text("xxxx", get_stc()->get_vi().get_macros().get(), s));
    REQUIRE(!wex::autocomplete_text("Date", // not unique!
      get_stc()->get_vi().get_macros().get(), s));
    REQUIRE( wex::autocomplete_text("Datet", get_stc()->get_vi().get_macros().get(), s));
    REQUIRE( s == "Datetime");
  }
  
  SUBCASE("autocomplete_filename")
  {
    REQUIRE( std::get<0> (wex::autocomplete_filename("te")));
    REQUIRE( std::get<1> (wex::autocomplete_filename("te")) == "st");
    REQUIRE(!std::get<0> (wex::autocomplete_filename("XX")));
    
#ifdef __UNIX__
#ifndef __WXOSX__    
    REQUIRE( std::get<0> (wex::autocomplete_filename("/usr/include/s")));
    REQUIRE( std::get<0> (wex::autocomplete_filename("../../../src/src/v")));
    // It is not clear whether ~ is relative or absolute...
    //REQUIRE( wex::autocomplete_filename("~/", expansion, v));
#endif    
#endif
  }
  
  SUBCASE("before")
  {
    REQUIRE( wex::before("nospace", ' ', false) == "nospace");
    REQUIRE( wex::before("nospace", ' ', true) == "nospace");
    REQUIRE( wex::before("some space and more", ' ', false) == "some space and");
    REQUIRE( wex::before("some space and more", ' ', true) == "some");
    REQUIRE( wex::before("some space and more", 'm', false) == "some space and ");
  }

  SUBCASE("browser_search")
  {
    // Causes travis to hang.
    // REQUIRE( wex::browser_search("test"));
  }

  SUBCASE("clipboard_add")
  {
    REQUIRE( wex::clipboard_add("test"));
  }
  
  SUBCASE("clipboard_get")
  {
    REQUIRE( wex::clipboard_get() == "test");
  }
  
  SUBCASE("combobox_as")
  {
    auto* cb = new wxComboBox(frame(), wxID_ANY);
    wex::test::add_pane(frame(), cb);
    wex::combobox_as<const std::list < std::string >>(cb, l);
  }
  
  SUBCASE("combobox_from_list")
  {
    auto* cb = new wxComboBox(frame(), wxID_ANY);
    wex::test::add_pane(frame(), cb);

    wex::combobox_from_list(cb, l);
    REQUIRE( cb->GetCount() == 3);
  }
  
  SUBCASE("comparefile")
  {
    wex::config(_("list.Comparator")).set("diff");

    REQUIRE( wex::comparefile(
      wex::test::get_path("test.h"), wex::test::get_path("test.h")));
  }
  
  SUBCASE("ellipsed")
  {
    REQUIRE( wex::ellipsed("xxx").find("...") != std::string::npos);
  }
  
  SUBCASE("firstof")
  {
    REQUIRE( wex::firstof("this is ok", "x") == std::string());
    REQUIRE( wex::firstof("this is ok", " ;,") == "is ok");
    REQUIRE( wex::firstof("this is ok", " ;,i") == "s is ok");
    REQUIRE( wex::firstof("this is ok", " ;,i", std::string::npos, 
      wex::firstof_t().set(wex::FIRST_OF_FROM_END)) == "ok");
    REQUIRE( wex::firstof("this is ok", " ", 0, 
      wex::firstof_t().set(wex::FIRST_OF_BEFORE)) == "this");
    REQUIRE( wex::firstof("this is ok", "x", 0, 
      wex::firstof_t().set(wex::FIRST_OF_BEFORE)) == "this is ok");
  }

  SUBCASE("get_endoftext")
  {
    REQUIRE( wex::get_endoftext("test", 3).size() == 3);
    REQUIRE( wex::get_endoftext("testtest", 3).size() == 3);
  }
  
  SUBCASE("get_find_result")
  {
    REQUIRE( wex::get_find_result("test", true, true).find("test") != std::string::npos);
    REQUIRE( wex::get_find_result("test", true, false).find("test") != std::string::npos);
    REQUIRE( wex::get_find_result("test", false, true).find("test") != std::string::npos);
    REQUIRE( wex::get_find_result("test", false, false).find("test") != std::string::npos);
    
    REQUIRE( wex::get_find_result("%d", true, true).find("%d") != std::string::npos);
    REQUIRE( wex::get_find_result("%d", true, false).find("%d") != std::string::npos);
    REQUIRE( wex::get_find_result("%d", false, true).find("%d") != std::string::npos);
    REQUIRE( wex::get_find_result("%d", false, false).find("%d") != std::string::npos);
  }
  
  SUBCASE("get_iconid")
  {
    REQUIRE( wex::get_iconid( wex::test::get_path("test.h")) != -1);
  }

  SUBCASE("get_number_of_lines  ")
  {
    REQUIRE( wex::get_number_of_lines("test") == 1);
    REQUIRE( wex::get_number_of_lines("test\n") == 2);
    REQUIRE( wex::get_number_of_lines("test\ntest") == 2);
    REQUIRE( wex::get_number_of_lines("test\ntest\n") == 3);
    REQUIRE( wex::get_number_of_lines("test\rtest\r") == 3);
    REQUIRE( wex::get_number_of_lines("test\r\ntest\n") == 3);
    
    REQUIRE( wex::get_number_of_lines("test\r\ntest\n\n\n", true) == 2);
    REQUIRE( wex::get_number_of_lines("test\r\ntest\n\n", true) == 2);
    REQUIRE( wex::get_number_of_lines("test\r\ntest\n\n", true) == 2);
  }
  
  SUBCASE("get_string_set")
  {
    REQUIRE( wex::get_string_set({"one", "two", "three"}) == "one three two ");
    REQUIRE( wex::get_string_set({"one", "two", "three"}, 4) == "three ");
  }

  SUBCASE("get_time")
  {
    const auto& [r, t] = wex::get_time("2019-02-01 12:20:06", "%Y-%m-%d %H:%M:%S");
    REQUIRE ( r );
    
    const auto& [r2, t2] = wex::get_time("201902-01 12:20:06", "%Y-%m-%d %H:%M:%S");
    REQUIRE ( !r2 );
  }

  SUBCASE("get_word")
  {
    std::string word("this is a test");
    REQUIRE( wex::get_word(word) == "this");
    REQUIRE( wex::get_word(word) == "is");
    REQUIRE( wex::get_word(word) == "a");
  }
  
  SUBCASE("is_brace")
  {
    for (const auto& c : cs)
    {
      REQUIRE( wex::is_brace(c));
    }
    
    REQUIRE(!wex::is_brace('a'));
  }

  SUBCASE("is_codeword_separator")
  {
    cs.insert(cs.end(), {',',';',':','@'});
    
    for (const auto& c : cs)
    {
      REQUIRE( wex::is_codeword_separator(c));
    }
    
    REQUIRE(!wex::is_codeword_separator('x'));
  }
  
#ifdef __UNIX__
  SUBCASE("make")
  {
    wex::path cwd; // as /usr/bin/git changes wd
    REQUIRE( wex::make(wex::path("xxx")) != -1);
    REQUIRE( wex::make(wex::path("make.tst")) != -1);
    REQUIRE( wex::make(wex::path("/usr/bin/git")) != -1);
  }
#endif
  
  SUBCASE("match")
  {
    std::vector<std::string> v;
    REQUIRE( wex::match("hllo", "hello world", v) == -1);
    REQUIRE( wex::match("hello", "hello world", v) == 0);
    REQUIRE( wex::match("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wex::match("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wex::match(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
    REQUIRE( wex::match("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  }
  
  SUBCASE("matches_one_of")
  {
    REQUIRE(!wex::matches_one_of("test.txt", "*.cpp"));
    REQUIRE( wex::matches_one_of("test.txt", "*.txt"));
    REQUIRE( wex::matches_one_of("test.txt", "*.cpp;*.txt"));
  }
  
  SUBCASE("node_properties")
  {
  }
  
  SUBCASE("node_styles")
  {
  }
  
  SUBCASE("open_files")
  {
    wex::path::current(wex::test::get_path().string());

    get_stc()->SetFocus();
    get_stc()->DiscardEdits();

    REQUIRE( wex::open_files(
      frame(), std::vector<wex::path>()) == 0);
    REQUIRE( wex::open_files(
      frame(), 
      std::vector<wex::path> {
        wex::test::get_path("test.h").data(), 
        "test.cpp", 
        "*xxxxxx*.cpp"}) == 2);
    REQUIRE( wex::open_files(
      frame(), 
      std::vector<wex::path> {wex::test::get_path("test.h").data()}) == 1);
    REQUIRE( wex::open_files(
      frame(), std::vector<wex::path> {"../../data/wex-menus.xml"}) == 1);
  }

  SUBCASE("open_files_dialog")
  {
  }
  
  SUBCASE("print_caption")
  {
    REQUIRE( wex::print_caption(
      wex::path("test")).find("test") != std::string::npos);
  }
  
  SUBCASE("print_footer")
  {
    REQUIRE( wex::print_footer().find("@") != std::string::npos);
  }
  
  SUBCASE("print_header")
  {
    REQUIRE( wex::print_header(
      wex::test::get_path("test.h")).find("test") != std::string::npos);
  }
  
  SUBCASE("marker_and_register_expansion")
  {
    get_stc()->set_text("this is some text");
    auto* ex = new wex::ex(get_stc());
    std::string command("xxx");
    REQUIRE(!wex::marker_and_register_expansion(nullptr, command));
    REQUIRE( wex::marker_and_register_expansion(ex, command));
    command = "'yxxx";
    REQUIRE(!wex::marker_and_register_expansion(ex, command));
    wex::clipboard_add("yanked");
    command = "this is * end";
    REQUIRE( wex::marker_and_register_expansion(ex, command));
#ifndef __WXMSW__    
    REQUIRE( command == "this is yanked end");
#endif
  }
  
  SUBCASE("quoted")
  {
    REQUIRE( wex::quoted("test") == "'test'");
    REQUIRE( wex::quoted("%d") == "'%d'");
    REQUIRE( wex::quoted(wex::trim(" %d ")) == "'%d'");
  }
  
  SUBCASE("replace_all")
  {
    int match_pos;
    const std::string org("test x y z x y z");
    std::string text(org);

    REQUIRE( wex::replace_all(text, "x", "aha", &match_pos) == 2);
    REQUIRE( match_pos == 5);

    text = org;
    REQUIRE( wex::replace_all(text, "xy", "aha", &match_pos) == 0);
    REQUIRE( match_pos == 5);
  }

#ifdef __UNIX__
  SUBCASE("shell_expansion")
  {
    std::string command("xxx `pwd` `pwd`");
    REQUIRE( wex::shell_expansion(command));
    REQUIRE( command.find("`") == std::string::npos);
    command = "no quotes";
    REQUIRE( wex::shell_expansion(command));
    REQUIRE( command == "no quotes");
    command = "illegal process `xyz`";
    REQUIRE(!wex::shell_expansion(command));
    REQUIRE( command == "illegal process `xyz`");
  }
#endif

  SUBCASE("sort")
  {
    REQUIRE(wex::sort("z\ny\nx\n", 0, 0, "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort("z\ny\nx\n", 
      wex::string_sort_t().set(wex::STRING_SORT_DESCENDING), 0, "\n") == "z\ny\nx\n");
    REQUIRE(wex::sort("z\nz\ny\nx\n", 0, 0, "\n") == "x\ny\nz\nz\n");
    REQUIRE(wex::sort("z\nz\ny\nx\n", 
      wex::string_sort_t().set(wex::STRING_SORT_UNIQUE), 0, "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort(rect, 0, 3, "\n", 5) == sorted);
  }

  SUBCASE("sort_selection")
  {
    get_stc()->SelectNone();
    REQUIRE(!wex::sort_selection(get_stc()));
    get_stc()->set_text("aaaaa\nbbbbb\nccccc\n");
    get_stc()->SelectAll();
    wxMilliSleep(10);
    REQUIRE( wex::sort_selection(get_stc()));
    wxMilliSleep(10);
    REQUIRE( wex::sort_selection(get_stc(), 0, 3, 10));
    wxMilliSleep(10);
    REQUIRE(!wex::sort_selection(get_stc(), 0, 20, 10));
  }

  SUBCASE("sort_selection_rect")
  {
    get_stc()->get_vi().command("\x1b");
    get_stc()->SelectNone();
    get_stc()->set_text(rect);

    // make a rectangular selection, invoke sort, and check result
    REQUIRE( get_stc()->get_vi().mode().normal());
    REQUIRE( get_stc()->get_vi().command("3 "));
    REQUIRE( get_stc()->get_vi().command("K"));
    REQUIRE( get_stc()->get_vi().mode().visual());
    REQUIRE( get_stc()->get_vi().command("4j"));
    REQUIRE( get_stc()->get_vi().command("5l"));
    REQUIRE( get_stc()->get_vi().mode().visual());

    REQUIRE( wex::sort_selection(get_stc(), 0, 3, 5));
    REQUIRE( wex::trim(get_stc()->GetText()) == wex::trim(sorted));
    REQUIRE( wex::sort_selection(get_stc(), 
      wex::string_sort_t().set(wex::STRING_SORT_DESCENDING), 3, 5));
    REQUIRE( get_stc()->GetText() != sorted);
  }
  
  SUBCASE("translate")
  {
    REQUIRE(wex::translate(
      "hello @PAGENUM@ from @PAGESCNT@", 1, 2).find("@") == std::string::npos);
  }

  SUBCASE("trim")
  {
    REQUIRE( wex::trim("\n\tt \n    es   t\n", wex::skip_t().set()) == "t es t");
    REQUIRE( wex::trim("\n\tt \n    es   t\n", 
      wex::skip_t().set(wex::TRIM_LEFT)) == "t \n    es   t\n");
    REQUIRE( wex::trim("\n\tt \n    es   t\n", 
      wex::skip_t().set(wex::TRIM_RIGHT)) == "\n\tt \n    es   t");
    REQUIRE( wex::trim("\n\tt \n    es   t\n", 
      wex::skip_t().set(wex::TRIM_LEFT).set(wex::TRIM_RIGHT)) ==  "t \n    es   t");
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
    // wex::vcs_execute(frame(), 0, std::vector< std::string > {}); // calls dialog
  }

  SUBCASE("xml_error")
  {
    wex::path fn("xml-err.xml");
    pugi::xml_parse_result pr;
    pr.status = pugi::xml_parse_status::status_ok;
    wex::xml_error(fn, &pr);
  }
}
