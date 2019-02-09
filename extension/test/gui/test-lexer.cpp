////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/lexer.h>
#include <wex/managedframe.h>
#include <wex/lexers.h>
#include <wex/stc.h>
#include "test.h"

TEST_CASE("wex::lexer")
{
  wex::lexer lexer;

  SUBCASE("Default constructor")
  {
    REQUIRE(!lexer.is_ok());
    REQUIRE(!lexer.previewable());
    REQUIRE( lexer.styles().empty());
    REQUIRE( lexer.display_lexer().empty());
    REQUIRE( lexer.scintilla_lexer().empty());
    REQUIRE( lexer.line_size() > 0);
  }

  SUBCASE("Default constructor with lexer")
  {
    REQUIRE( wex::lexer("cpp").is_ok());
    REQUIRE( wex::lexer("pascal").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());
    REQUIRE(!wex::lexer().set("xxx"));
  }
  
  SUBCASE("Constructor using STC")
  {
    wex::lexer lexer(get_stc());
    REQUIRE(!lexer.is_ok());
    lexer.apply();
  }

  SUBCASE("Set")
  {
    REQUIRE( lexer.set("xsl"));
    REQUIRE( lexer.language() == "xml");

    REQUIRE( lexer.set("pascal"));
    wex::lexer lexer2;
    REQUIRE( lexer2.set(lexer));
    REQUIRE( lexer2.set(lexer, true));
    REQUIRE( lexer2.display_lexer() == "pascal");
    REQUIRE( lexer2.scintilla_lexer() == "pascal");

    REQUIRE(!lexer.set(wex::lexers::get()->find_by_text("XXXX")));
    REQUIRE( lexer.display_lexer().empty());
    REQUIRE(!lexer.is_ok());
    REQUIRE( lexer.set(wex::lexers::get()->find_by_text("<html>")));
    REQUIRE( lexer.is_ok());
    REQUIRE( lexer.scintilla_lexer() == "hypertext");
    REQUIRE( lexer.display_lexer() == "hypertext");
    REQUIRE( lexer.previewable());
    REQUIRE( lexer.set(wex::lexers::get()->find_by_text("// this is a cpp comment text")));
    REQUIRE( lexer.is_ok());
    REQUIRE( wex::lexer(lexer).is_ok());
    REQUIRE( lexer.display_lexer() == "cpp");
    REQUIRE( lexer.scintilla_lexer() == "cpp");
  }

  SUBCASE("Reset")
  {
    lexer.set("markdown");
    REQUIRE( lexer.edge_mode() == wex::edge_mode_t::NONE);
    lexer.reset();
    REQUIRE( lexer.display_lexer().empty());
    REQUIRE( lexer.scintilla_lexer().empty());
    REQUIRE( lexer.edge_mode() == wex::edge_mode_t::ABSENT);
  }

  SUBCASE("Testing several methods")
  {
    REQUIRE( lexer.set("cpp"));
    REQUIRE( lexer.display_lexer() == "cpp");
    REQUIRE( lexer.scintilla_lexer() == "cpp");
    REQUIRE( lexer.usable_chars_per_line() > 0);
    REQUIRE(!lexer.extensions().empty());
    REQUIRE(!lexer.comment_begin().empty());
    REQUIRE(!lexer.comment_begin2().empty());
    REQUIRE( lexer.comment_end().empty());
    REQUIRE(!lexer.comment_end2().empty());
    REQUIRE( lexer.language().empty());
    REQUIRE(!lexer.keywords().empty());
    REQUIRE(!lexer.styles().empty());
    REQUIRE(!lexer.keywords_string().empty());
    REQUIRE(!lexer.keywords_string(-1, 0).empty());
    REQUIRE(!lexer.keywords_string(-1, 6).empty());
    REQUIRE( lexer.keywords_string(-1, 8).find("for_each") != std::string::npos);
    REQUIRE( lexer.keywords_string(-1, 9).find("for_each") == std::string::npos);
    REQUIRE( lexer.keywords_string(-1, 50).empty());
    REQUIRE( lexer.comment_complete("// test").empty());
    REQUIRE( lexer.is_keyword("class"));
    REQUIRE( lexer.is_keyword("const"));
    REQUIRE( lexer.keyword_starts_with("cla"));
    REQUIRE(!lexer.keyword_starts_with("xxx"));
    REQUIRE(!lexer.make_comment("test", true).empty());
    REQUIRE(!lexer.make_comment("prefix", "test").empty());
    REQUIRE(!lexer.make_comment("test\ntest2", true).empty());
    REQUIRE(!lexer.make_comment("prefix", "test\ntest2").empty());
    REQUIRE(!lexer.make_single_line_comment("test").empty());
    REQUIRE( lexer.keywords_string(6).empty());
    REQUIRE( lexer.add_keywords("hello:1"));
    REQUIRE( lexer.add_keywords("more:1"));
    REQUIRE( lexer.add_keywords("test11 test21:1 test31:1 test12:2 test22:2"));
    REQUIRE( lexer.add_keywords("final", 6));
    REQUIRE(!lexer.add_keywords(""));
    REQUIRE(!lexer.add_keywords("xxx:1", -1));
    REQUIRE(!lexer.add_keywords("xxx:1", 100));
    REQUIRE(!lexer.keywords_string(6).empty());
    REQUIRE( lexer.is_keyword("hello")); 
    REQUIRE( lexer.is_keyword("more")); 
    REQUIRE( lexer.is_keyword("class")); 
    REQUIRE( lexer.is_keyword("test11"));
    REQUIRE( lexer.is_keyword("test21"));
    REQUIRE( lexer.is_keyword("test12"));
    REQUIRE( lexer.is_keyword("test22"));
    REQUIRE( lexer.is_keyword("test31"));
    REQUIRE( lexer.is_keyword("final"));
    REQUIRE(!lexer.is_keyword("xxx"));
    REQUIRE( lexer.keyword_starts_with("te"));
    REQUIRE(!lexer.keyword_starts_with("xx"));
    REQUIRE(!lexer.keywords().empty());

    lexer.apply();
  }
  
  SUBCASE("Property")
  {
    lexer.set_property("test", "value");
    lexer.set_property("other", "one");

    REQUIRE( lexer.properties().front().GetValue() == "value");
    REQUIRE( lexer.properties().back().GetValue() == "one");
  }

  SUBCASE("Comment complete")
  {
    REQUIRE( lexer.set("pascal"));
    REQUIRE( lexer.display_lexer() == "pascal");
    REQUIRE( lexer.scintilla_lexer() == "pascal");
    REQUIRE( std::regex_match(lexer.comment_complete("(*test"), std::regex(" +\\*\\)")));
  }
  
  SUBCASE("lexers")
  {
    REQUIRE( wex::lexer("ada").is_ok());
    REQUIRE( wex::lexer("rfw").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());
  }
}
