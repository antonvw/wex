////////////////////////////////////////////////////////////////////////////////
// Name:      test-util.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/util.h>
#include <wx/extension/ex.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vcscommand.h>
#include <wx/extension/vi-macros.h>
#include "test.h"

TEST_CASE("wex")
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

  SUBCASE("wex::after")
  {
    REQUIRE( wex::after("nospace", ' ', false) == "nospace");
    REQUIRE( wex::after("nospace", ' ', true) == "nospace");
    REQUIRE( wex::after("some space and more", ' ', false) == "more");
    REQUIRE( wex::after("some space and more", ' ', true) == "space and more");
    REQUIRE( wex::after("some space and more", 'm', false) == "ore");
  }

  SUBCASE("wex::align_text")
  {
    REQUIRE( wex::align_text("test", "header", true, true,
      wex::lexers::Get()->FindByName("cpp")).size() 
        == std::string("// headertest").size());
  }
      
  SUBCASE("wex::autocomplete_text")
  {
    REQUIRE( wex::vi_macros::LoadDocument());
    std::string s;
    REQUIRE(!wex::autocomplete_text("xxxx", GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE(!wex::autocomplete_text("Date", // not unique!
      GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE( wex::autocomplete_text("Datet", GetSTC()->GetVi().GetMacros().Get(), s));
    REQUIRE( s == "Datetime");
  }
  
  SUBCASE("wex::autocomplete_filename")
  {
    REQUIRE( std::get<0> (wex::autocomplete_filename("te")));
    REQUIRE( std::get<1> (wex::autocomplete_filename("te")) == "st");
    REQUIRE(!std::get<0> (wex::autocomplete_filename("XX")));
    
#ifdef __UNIX__
#ifndef __WXOSX__    
    REQUIRE( std::get<0> (wex::autocomplete_filename("/usr/include/s")));
    REQUIRE( std::get<0> (wex::autocomplete_filename("../../../extension/src/v")));
    // It is not clear whether ~ is relative or absolute...
    //REQUIRE( wex::autocomplete_filename("~/", expansion, v));
#endif    
#endif
  }
  
  SUBCASE("wex::before")
  {
    REQUIRE( wex::before("nospace", ' ', false) == "nospace");
    REQUIRE( wex::before("nospace", ' ', true) == "nospace");
    REQUIRE( wex::before("some space and more", ' ', false) == "some space and");
    REQUIRE( wex::before("some space and more", ' ', true) == "some");
    REQUIRE( wex::before("some space and more", 'm', false) == "some space and ");
  }

  SUBCASE("wex::browser_search")
  {
    // Causes travis to hang.
    // REQUIRE( wex::browser_search("test"));
  }

  SUBCASE("wex::clipboard_add")
  {
    REQUIRE( wex::clipboard_add("test"));
  }
  
  SUBCASE("wex::clipboard_get")
  {
    REQUIRE( wex::clipboard_get() == "test");
  }
  
  SUBCASE("wex::combobox_as")
  {
    wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
#ifndef __WXOSX__
    AddPane(GetFrame(), cb);
#endif
    wex::combobox_as<const std::list < std::string >>(cb, l);
  }
  
  SUBCASE("wex::combobox_from_list")
  {
    wxComboBox* cb = new wxComboBox(GetFrame(), wxID_ANY);
#ifndef __WXOSX__
    AddPane(GetFrame(), cb);
#endif
    wex::combobox_from_list(cb, l);
    REQUIRE( cb->GetCount() == 3);
  }
  
  SUBCASE("wex::comparefile")
  {
  }
  
  SUBCASE("wex::config_dir")
  {
#ifdef __WXMSW__
    REQUIRE(!wex::config_dir().empty());
#else
    REQUIRE(wex::config_dir().find(".config") != std::string::npos);
#endif
  }
  
  SUBCASE("wex::config_firstof")
  {
    wex::config_firstof("xxxx");
  }
  
  SUBCASE("wex::config_firstof_write")
  {
    REQUIRE( wex::config_firstof_write("xxxx", "zz") == "zz");
  }
  
  SUBCASE("wex::ellipsed")
  {
    REQUIRE( wex::ellipsed("xxx").find("...") != std::string::npos);
  }
  
  SUBCASE("wex::firstof")
  {
    REQUIRE( wex::firstof("this is ok", "x") == std::string());
    REQUIRE( wex::firstof("this is ok", " ;,") == "is ok");
    REQUIRE( wex::firstof("this is ok", " ;,i") == "s is ok");
    REQUIRE( wex::firstof("this is ok", " ;,i", std::string::npos, wex::FIRST_OF_FROM_END) == "ok");
    REQUIRE( wex::firstof("this is ok", " ", 0, wex::FIRST_OF_BEFORE) == "this");
    REQUIRE( wex::firstof("this is ok", "x", 0, wex::FIRST_OF_BEFORE) == "this is ok");
  }

  SUBCASE("wex::get_endoftext")
  {
    REQUIRE( wex::get_endoftext("test", 3).size() == 3);
    REQUIRE( wex::get_endoftext("testtest", 3).size() == 3);
  }
  
  SUBCASE("wex::get_field_separator")
  {
    REQUIRE((wex::get_field_separator() != 'a'));
  }

  SUBCASE("wex::get_find_result")
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
  
  SUBCASE("wex::get_iconid")
  {
    REQUIRE( wex::get_iconid( GetTestPath("test.h")) != -1);
  }

  SUBCASE("wex::get_number_of_lines  ")
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
  
  SUBCASE("wex::get_string_set")
  {
    REQUIRE( wex::get_string_set({"one", "two", "three"}) == "one three two ");
    REQUIRE( wex::get_string_set({"one", "two", "three"}, 4) == "three ");
  }

  SUBCASE("wex::get_word")
  {
  }
  
  SUBCASE("wex::is_brace")
  {
    for (const auto& c : cs)
    {
      REQUIRE( wex::is_brace(c));
    }
    
    REQUIRE(!wex::is_brace('a'));
  }

  SUBCASE("wex::is_codeword_separator")
  {
    cs.insert(cs.end(), {',',';',':','@'});
    
    for (const auto& c : cs)
    {
      REQUIRE( wex::is_codeword_separator(c));
    }
    
    REQUIRE(!wex::is_codeword_separator('x'));
  }
  
  SUBCASE("wex::list_from_config")
  {
    REQUIRE( wex::list_from_config("xxx").size() == 0);
  }
  
  SUBCASE("wex::list_to_config")
  {
    l.clear();
    l.emplace_back("1");
    l.emplace_back("2");
    wex::list_to_config(l, "list_items");
    REQUIRE( l.size() == 2);
    REQUIRE(wxConfigBase::Get()->Read("list_items", "").Contains("1"));
    REQUIRE(wxConfigBase::Get()->Read("list_items", "").Contains("2"));
  }
  
  SUBCASE("wex::log_status")
  {
    wex::log_status( GetTestPath("test.h"));
    wex::log_status( std::string("hello world") );
  }

#ifdef __UNIX__
  SUBCASE("wex::make")
  {
    wex::path cwd; // as /usr/bin/git changes wd
    REQUIRE( wex::make(wex::path("xxx")) != -1);
    REQUIRE( wex::make(wex::path("make.tst")) != -1);
    REQUIRE( wex::make(wex::path("/usr/bin/git")) != -1);
  }
#endif
  
  SUBCASE("wex::match")
  {
    std::vector<std::string> v;
    REQUIRE( wex::match("hllo", "hello world", v) == -1);
    REQUIRE( wex::match("hello", "hello world", v) == 0);
    REQUIRE( wex::match("([0-9]+)ok([0-9]+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wex::match("(\\d+)ok(\\d+)nice", "19999ok245nice", v) == 2);
    REQUIRE( wex::match(" ([\\d\\w]+)", " 19999ok245nice ", v) == 1);
    REQUIRE( wex::match("([?/].*[?/])(,[?/].*[?/])([msy])", "/xx/,/yy/y", v) == 3);
  }
  
  SUBCASE("wex::matches_one_of")
  {
    REQUIRE(!wex::matches_one_of("test.txt", "*.cpp"));
    REQUIRE( wex::matches_one_of("test.txt", "*.txt"));
    REQUIRE( wex::matches_one_of("test.txt", "*.cpp;*.txt"));
  }
  
  SUBCASE("wex::node_properties")
  {
  }
  
  SUBCASE("wex::node_styles")
  {
  }
  
  SUBCASE("wex::open_files")
  {
    REQUIRE( wex::open_files(GetFrame(), std::vector<wex::path>()) == 0);
    REQUIRE( wex::open_files(GetFrame(), std::vector<wex::path> {
      GetTestPath("test.h").Path(), "test.cpp", "*xxxxxx*.cpp"}) == 2);
    REQUIRE( wex::open_files(GetFrame(), 
      std::vector<wex::path> {GetTestPath("test.h").Path()}) == 1);
    REQUIRE( 
      wex::open_files(GetFrame(), std::vector<wex::path> {"../../data/menus.xml"}) == 1);
  }

  SUBCASE("wex::open_files_dialog")
  {
  }
  
  SUBCASE("wex::print_caption")
  {
    REQUIRE( wex::print_caption(wex::path("test")).find("test") != std::string::npos);
  }
  
  SUBCASE("wex::print_footer")
  {
    REQUIRE( wex::print_footer().find("@") != std::string::npos);
  }
  
  SUBCASE("wex::print_header")
  {
    REQUIRE( wex::print_header(GetTestPath("test.h")).find("test") != std::string::npos);
  }
  
  SUBCASE("wex::marker_and_register_expansion")
  {
    GetSTC()->SetText("this is some text");
    wex::ex* ex = new wex::ex(GetSTC());
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
  
  SUBCASE("wex::quoted")
  {
    REQUIRE( wex::quoted("test") == "'test'");
    REQUIRE( wex::quoted("%d") == "'%d'");
    REQUIRE( wex::quoted(wex::skip_white_space(" %d ")) == "'%d'");
  }
  
  SUBCASE("wex::replace_all")
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
#ifndef __WXOSX__
  SUBCASE("wex::shell_expansion")
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
#endif
  
  SUBCASE("wex::sort")
  {
    REQUIRE(wex::sort("z\ny\nx\n", wex::STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort("z\ny\nx\n", wex::STRING_SORT_DESCENDING, 0, "\n") == "z\ny\nx\n");
    REQUIRE(wex::sort("z\nz\ny\nx\n", wex::STRING_SORT_ASCENDING, 0, "\n") == "x\ny\nz\nz\n");
    REQUIRE(wex::sort("z\nz\ny\nx\n", wex::STRING_SORT_ASCENDING | wex::STRING_SORT_UNIQUE, 0, "\n") == "x\ny\nz\n");
    REQUIRE(wex::sort(rect, wex::STRING_SORT_ASCENDING, 3, "\n", 5) == sorted);
  }

  SUBCASE("wex::sort_selection")
  {
    GetSTC()->SelectNone();
    REQUIRE( wex::sort_selection(GetSTC()));
    GetSTC()->SetText("aaaaa\nbbbbb\nccccc\n");
    GetSTC()->SelectAll();
    REQUIRE( wex::sort_selection(GetSTC()));
    REQUIRE( wex::sort_selection(GetSTC(), wex::STRING_SORT_ASCENDING, 3, 10));
    REQUIRE(!wex::sort_selection(GetSTC(), wex::STRING_SORT_ASCENDING, 20, 10));
    GetSTC()->SelectNone();
    GetSTC()->SetText(rect);
    // force rectangular selection.
    (void)GetSTC()->GetVi().Command("3 ");
    (void)GetSTC()->GetVi().Command("K");
    (void)GetSTC()->GetVi().Command("4j");
    (void)GetSTC()->GetVi().Command("5l");
    REQUIRE( wex::sort_selection(GetSTC(), wex::STRING_SORT_ASCENDING, 3, 5));
#ifdef __WXGTK__
    REQUIRE( wex::skip_white_space(GetSTC()->GetText().ToStdString()) == wex::skip_white_space(sorted));
#endif
    REQUIRE( wex::sort_selection(GetSTC(), wex::STRING_SORT_DESCENDING, 3, 5));
    REQUIRE( GetSTC()->GetText() != sorted);
  }
  
  SUBCASE("wex::skip_white_space")
  {
    REQUIRE( wex::skip_white_space("\n\tt \n    es   t\n", wex::SKIP_ALL) == "t es t");
    REQUIRE( wex::skip_white_space("\n\tt \n    es   t\n", wex::SKIP_LEFT) == "t \n    es   t\n");
    REQUIRE( wex::skip_white_space("\n\tt \n    es   t\n", wex::SKIP_RIGHT) == "\n\tt \n    es   t");
    REQUIRE( wex::skip_white_space("\n\tt \n    es   t\n", wex::SKIP_BOTH) == "t \n    es   t");
  }
  
  SUBCASE("wex::translate")
  {
    REQUIRE(wex::translate(
      "hello @PAGENUM@ from @PAGESCNT@", 1, 2).find("@") == std::string::npos);
  }

  SUBCASE("wex::vcs_command_stc")
  {
    wex::vcs_command command("status");
    wex::vcs_command_stc(command, wex::lexer(GetSTC(), "cpp"), GetSTC());
    wex::vcs_command_stc(command, wex::lexer(nullptr, "cpp"), GetSTC());
    wex::vcs_command_stc(command, wex::lexer(), GetSTC());
  }
  
  SUBCASE("wex::vcs_execute")
  {
    // wex::vcs_execute(GetFrame(), 0, std::vector< wxString > {}); // calls dialog
  }

  SUBCASE("wex::xml_error")
  {
    wex::path fn("xml-err.xml");
    pugi::xml_parse_result pr;
    pr.status = pugi::xml_parse_status::status_ok;
    wex::xml_error(fn, &pr);
  }
}
