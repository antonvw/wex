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
    REQUIRE(!lexer.Previewable());
    REQUIRE( lexer.GetStyles().empty());
    REQUIRE( lexer.GetDisplayLexer().empty());
    REQUIRE( lexer.GetScintillaLexer().empty());
    REQUIRE( lexer.GetLineSize() > 0);
  }

  SUBCASE("Default constructor with lexer")
  {
    REQUIRE( wex::lexer("cpp").is_ok());
    REQUIRE( wex::lexer("pascal").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());
    REQUIRE(!wex::lexer().Set("xxx"));
  }
  
  SUBCASE("Constructor using STC")
  {
    wex::lexer lexer(GetSTC());
    REQUIRE(!lexer.is_ok());
    lexer.Apply();
  }

  SUBCASE("Set")
  {
    REQUIRE( lexer.Set("xsl"));
    REQUIRE( lexer.GetLanguage() == "xml");

    REQUIRE( lexer.Set("pascal"));
    wex::lexer lexer2;
    REQUIRE( lexer2.Set(lexer));
    REQUIRE( lexer2.Set(lexer, true));
    REQUIRE( lexer2.GetDisplayLexer() == "pascal");
    REQUIRE( lexer2.GetScintillaLexer() == "pascal");

    REQUIRE(!lexer.Set(wex::lexers::Get()->FindByText("XXXX")));
    REQUIRE( lexer.GetDisplayLexer().empty());
    REQUIRE(!lexer.is_ok());
    REQUIRE( lexer.Set(wex::lexers::Get()->FindByText("<html>")));
    REQUIRE( lexer.is_ok());
    REQUIRE( lexer.GetScintillaLexer() == "hypertext");
    REQUIRE( lexer.GetDisplayLexer() == "hypertext");
    REQUIRE( lexer.Previewable());
    REQUIRE( lexer.Set(wex::lexers::Get()->FindByText("// this is a cpp comment text")));
    REQUIRE( lexer.is_ok());
    REQUIRE( wex::lexer(lexer).is_ok());
    REQUIRE( lexer.GetDisplayLexer() == "cpp");
    REQUIRE( lexer.GetScintillaLexer() == "cpp");
  }

  SUBCASE("Reset")
  {
    lexer.Set("markdown");
    REQUIRE( lexer.GetEdgeMode() == wex::edge_mode::NONE);
    lexer.Reset();
    REQUIRE( lexer.GetDisplayLexer().empty());
    REQUIRE( lexer.GetScintillaLexer().empty());
    REQUIRE( lexer.GetEdgeMode() == wex::edge_mode::ABSENT);
  }

  SUBCASE("Testing several methods")
  {
    REQUIRE( lexer.Set("cpp"));
    REQUIRE( lexer.GetDisplayLexer() == "cpp");
    REQUIRE( lexer.GetScintillaLexer() == "cpp");
    REQUIRE( lexer.UsableCharactersPerLine() > 0);
    REQUIRE(!lexer.GetExtensions().empty());
    REQUIRE(!lexer.GetCommentBegin().empty());
    REQUIRE(!lexer.GetCommentBegin2().empty());
    REQUIRE( lexer.GetCommentEnd().empty());
    REQUIRE(!lexer.GetCommentEnd2().empty());
    REQUIRE( lexer.GetLanguage().empty());
    REQUIRE(!lexer.GetKeywords().empty());
    REQUIRE(!lexer.GetStyles().empty());
    REQUIRE(!lexer.GetKeywordsString().empty());
    REQUIRE(!lexer.GetKeywordsString(-1, 0).empty());
    REQUIRE(!lexer.GetKeywordsString(-1, 6).empty());
    REQUIRE( lexer.GetKeywordsString(-1, 8).find("for_each") != std::string::npos);
    REQUIRE( lexer.GetKeywordsString(-1, 9).find("for_each") == std::string::npos);
    REQUIRE( lexer.GetKeywordsString(-1, 50).empty());
    REQUIRE( lexer.CommentComplete("// test").empty());
    REQUIRE( lexer.IsKeyword("class"));
    REQUIRE( lexer.IsKeyword("const"));
    REQUIRE( lexer.KeywordStartsWith("cla"));
    REQUIRE(!lexer.KeywordStartsWith("xxx"));
    REQUIRE(!lexer.MakeComment("test", true).empty());
    REQUIRE(!lexer.MakeComment("prefix", "test").empty());
    REQUIRE(!lexer.MakeComment("test\ntest2", true).empty());
    REQUIRE(!lexer.MakeComment("prefix", "test\ntest2").empty());
    REQUIRE(!lexer.MakeSingleLineComment("test").empty());
    REQUIRE( lexer.GetKeywordsString(6).empty());
    REQUIRE( lexer.AddKeywords("hello:1"));
    REQUIRE( lexer.AddKeywords("more:1"));
    REQUIRE( lexer.AddKeywords("test11 test21:1 test31:1 test12:2 test22:2"));
    REQUIRE( lexer.AddKeywords("final", 6));
    REQUIRE(!lexer.AddKeywords(""));
    REQUIRE(!lexer.AddKeywords("xxx:1", -1));
    REQUIRE(!lexer.AddKeywords("xxx:1", 100));
    REQUIRE(!lexer.GetKeywordsString(6).empty());
    REQUIRE( lexer.IsKeyword("hello")); 
    REQUIRE( lexer.IsKeyword("more")); 
    REQUIRE( lexer.IsKeyword("class")); 
    REQUIRE( lexer.IsKeyword("test11"));
    REQUIRE( lexer.IsKeyword("test21"));
    REQUIRE( lexer.IsKeyword("test12"));
    REQUIRE( lexer.IsKeyword("test22"));
    REQUIRE( lexer.IsKeyword("test31"));
    REQUIRE( lexer.IsKeyword("final"));
    REQUIRE(!lexer.IsKeyword("xxx"));
    REQUIRE( lexer.KeywordStartsWith("te"));
    REQUIRE(!lexer.KeywordStartsWith("xx"));
    REQUIRE(!lexer.GetKeywords().empty());

    lexer.Apply();
  }
  
  SUBCASE("Property")
  {
    lexer.SetProperty("test", "value");
    lexer.SetProperty("other", "one");

    REQUIRE( lexer.GetProperties().front().GetValue() == "value");
    REQUIRE( lexer.GetProperties().back().GetValue() == "one");
  }

  SUBCASE("Comment complete")
  {
    REQUIRE( lexer.Set("pascal"));
    REQUIRE( lexer.GetDisplayLexer() == "pascal");
    REQUIRE( lexer.GetScintillaLexer() == "pascal");
    REQUIRE( std::regex_match(lexer.CommentComplete("(*test"), std::regex(" +\\*\\)")));
  }
  
  SUBCASE("lexers")
  {
    REQUIRE( wex::lexer("ada").is_ok());
    REQUIRE( wex::lexer("rfw").is_ok());
    REQUIRE(!wex::lexer("xxx").is_ok());
  }
}
