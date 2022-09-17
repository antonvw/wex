////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "../test.h"
#include <wex/syntax/lexer.h>

#include <regex>

TEST_CASE("wex::lexer")
{
  wex::lexer lexer;

  SUBCASE("default constructor")
  {
    REQUIRE(!lexer.is_ok());
    REQUIRE(!lexer.apply());
    REQUIRE(!lexer.is_previewable());
    REQUIRE(lexer.styles().empty());
    REQUIRE(lexer.display_lexer().empty());
    REQUIRE(lexer.scintilla_lexer().empty());
    REQUIRE(lexer.line_size() > 0);
  }

  SUBCASE("default constructor with lexer")
  {
    REQUIRE(wex::lexer("cpp").is_ok());
    REQUIRE(wex::lexer("pascal").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());
    REQUIRE(!wex::lexer().set("xxx"));
  }

  SUBCASE("constructor using xml")
  {
    pugi::xml_document doc;

    REQUIRE(doc.load_string("<lexer name=\"xyz\" tabwidth=\"12\"></lexer>"));

    auto       node = doc.document_element();
    wex::lexer lexer(&node);
    REQUIRE(lexer.is_ok());
    REQUIRE(lexer.scintilla_lexer() == "xyz");
    REQUIRE(lexer.display_lexer() == "xyz");
    REQUIRE(lexer.attrib(_("Tab width")) == 12);
  }

  SUBCASE("align_text")
  {
    REQUIRE(
      lexer.align_text("    code improvements", "", true, true)
        .find("    code") == std::string::npos);

    REQUIRE(
      wex::lexer("cpp").align_text("test", "header", true, true).size() ==
      std::string("// headertest").size());
  }

  SUBCASE("clear")
  {
    lexer.set("markdown");
    REQUIRE(!lexer.attribs().empty());
    REQUIRE(lexer.attrib(_("Edge line")) == wxSTC_EDGE_NONE);
    lexer.clear();
    REQUIRE(lexer.display_lexer().empty());
    REQUIRE(lexer.scintilla_lexer().empty());
    REQUIRE(lexer.attribs().empty());
    REQUIRE(lexer.attrib(_("Edge line")) == -1);
  }

  SUBCASE("comment_complete")
  {
    REQUIRE(lexer.set("pascal"));
    REQUIRE(lexer.display_lexer() == "pascal");
    REQUIRE(lexer.scintilla_lexer() == "pascal");
    REQUIRE(std::regex_match(
      lexer.comment_complete("(*test"),
      std::regex(" +\\*\\)")));
  }

  SUBCASE("make_comment")
  {
    REQUIRE(
      lexer.make_comment("commit xyz\n    code improvements")
        .find("code code improvements") == std::string::npos);

    REQUIRE(!lexer.make_comment("test", true).empty());
    REQUIRE(!lexer.make_comment("prefix", "test").empty());
    REQUIRE(!lexer.make_comment("test\ntest2", true).empty());
    REQUIRE(!lexer.make_comment("prefix", "test\ntest2").empty());
    REQUIRE(!lexer.make_single_line_comment("test").empty());
  }

  SUBCASE("property")
  {
    lexer.set_property("test", "value");
    lexer.set_property("other", "one");

    REQUIRE(lexer.properties().front().value() == "value");
    REQUIRE(lexer.properties().back().value() == "one");
  }

  SUBCASE("set")
  {
    REQUIRE(lexer.set("xsl"));
    REQUIRE(lexer.language() == "xml");

    REQUIRE(lexer.set("pascal"));
    wex::lexer lexer2;
    REQUIRE(lexer2.set(lexer));
    REQUIRE(lexer2.set(lexer, true));
    REQUIRE(lexer2.display_lexer() == "pascal");
    REQUIRE(lexer2.scintilla_lexer() == "pascal");

    REQUIRE(!lexer.set(wex::lexer("XXXX")));
    REQUIRE(lexer.display_lexer().empty());
    REQUIRE(!lexer.is_ok());
    REQUIRE(lexer.set(wex::lexer("hypertext")));
    REQUIRE(lexer.scintilla_lexer() == "hypertext");
    REQUIRE(lexer.display_lexer() == "hypertext");
    REQUIRE(lexer.is_previewable());
    REQUIRE(lexer.set(wex::lexer("cpp")));
    REQUIRE(lexer.is_ok());
    REQUIRE(wex::lexer(lexer).is_ok());
    REQUIRE(lexer.display_lexer() == "cpp");
    REQUIRE(lexer.scintilla_lexer() == "cpp");
  }

  SUBCASE("several methods")
  {
    REQUIRE(lexer.set("cpp"));
    REQUIRE(lexer.display_lexer() == "cpp");
    REQUIRE(lexer.scintilla_lexer() == "cpp");
    REQUIRE(lexer.usable_chars_per_line() > 0);
    REQUIRE(!lexer.extensions().empty());
    REQUIRE(!lexer.comment_begin().empty());
    REQUIRE(!lexer.comment_begin2().empty());
    REQUIRE(lexer.comment_end().empty());
    REQUIRE(!lexer.comment_end2().empty());
    REQUIRE(lexer.language().empty());
    REQUIRE(!lexer.keywords().empty());
    REQUIRE(!lexer.styles().empty());
    REQUIRE(!lexer.keywords_string().empty());
    REQUIRE(!lexer.keywords_string(-1, 0).empty());
    REQUIRE(!lexer.keywords_string(-1, 6).empty());
    REQUIRE(lexer.keywords_string(-1, 8).find("for_each") != std::string::npos);
    REQUIRE(lexer.keywords_string(-1, 9).find("for_each") == std::string::npos);
    REQUIRE(lexer.keywords_string(-1, 50).empty());
    REQUIRE(lexer.comment_complete("// test").empty());
    REQUIRE(lexer.is_keyword("class"));
    REQUIRE(lexer.is_keyword("const"));
    REQUIRE(lexer.keyword_starts_with("cla"));
    REQUIRE(!lexer.keyword_starts_with("xxx"));
    REQUIRE(lexer.keywords_string(6).empty());
    REQUIRE(lexer.add_keywords("hello:1"));
    REQUIRE(lexer.add_keywords("more:1"));
    REQUIRE(lexer.add_keywords("test11 test21:1 test31:1 test12:2 test22:2"));
    REQUIRE(lexer.add_keywords("final", 6));
    REQUIRE(!lexer.add_keywords(""));
    REQUIRE(!lexer.add_keywords("xxx:1", -1));
    REQUIRE(!lexer.add_keywords("xxx:1", 100));
    REQUIRE(!lexer.keywords_string(6).empty());
    REQUIRE(lexer.is_keyword("hello"));
    REQUIRE(lexer.is_keyword("more"));
    REQUIRE(lexer.is_keyword("class"));
    REQUIRE(lexer.is_keyword("test11"));
    REQUIRE(lexer.is_keyword("test21"));
    REQUIRE(lexer.is_keyword("test12"));
    REQUIRE(lexer.is_keyword("test22"));
    REQUIRE(lexer.is_keyword("test31"));
    REQUIRE(lexer.is_keyword("final"));
    REQUIRE(!lexer.is_keyword("xxx"));
    REQUIRE(lexer.keyword_starts_with("te"));
    REQUIRE(!lexer.keyword_starts_with("xx"));
    REQUIRE(!lexer.keywords().empty());
    REQUIRE(!lexer.apply());
  }
}
