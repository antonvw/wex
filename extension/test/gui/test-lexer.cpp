////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/lexer.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/lexers.h>
#include <wx/extension/stc.h>
#include "test.h"

TEST_CASE("wxExLexer", "[stc][lexer]")
{
  wxExSTC* stc = new wxExSTC(GetFrame(), "hello stc");
  AddPane(GetFrame(), stc);
  
  wxExLexer lexer;
  REQUIRE(!lexer.IsOk());
  REQUIRE( lexer.GetStyles().empty());
  
  REQUIRE( wxExLexer("cpp").IsOk());
  REQUIRE( wxExLexer("pascal").IsOk());
  REQUIRE(!wxExLexer("xxx").IsOk());
  
  lexer = wxExLexers::Get()->FindByText("XXXX");
  REQUIRE(!lexer.IsOk());
  
  lexer = wxExLexers::Get()->FindByText("<html>");
  REQUIRE( lexer.IsOk());
  REQUIRE( lexer.GetDisplayLexer() == "hypertext");
  
  lexer = wxExLexers::Get()->FindByText("// this is a cpp comment text");
  REQUIRE( lexer.IsOk());
  REQUIRE( wxExLexer(lexer).IsOk());
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
  REQUIRE( lexer.GetKeywordsString(-1, 8).Contains("for_each"));
  REQUIRE(!lexer.GetKeywordsString(-1, 9).Contains("for_each"));
  REQUIRE( lexer.GetKeywordsString(-1, 50).empty());
  REQUIRE( lexer.CommentComplete("// test").empty());

  REQUIRE( lexer.IsKeyword("class"));
  REQUIRE( lexer.IsKeyword("const"));

  REQUIRE( lexer.KeywordStartsWith("cla"));
  REQUIRE(!lexer.KeywordStartsWith("xxx"));

  REQUIRE(!lexer.MakeComment("test", true).empty());
  REQUIRE(!lexer.MakeComment("test", "test").empty());
  REQUIRE(!lexer.MakeSingleLineComment("test").empty());

  REQUIRE( lexer.GetKeywordsString(6).empty());
  REQUIRE( lexer.AddKeywords("hello:1"));
  REQUIRE( lexer.AddKeywords("more:1"));
  REQUIRE( lexer.AddKeywords(
    "test11 test21:1 test31:1 test12:2 test22:2"));
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
  
  lexer.SetProperty("test", "value");
  wxString val;

  for (auto p : lexer.GetProperties())
  {
    if (p.GetName() == "test")
    {
      val = p.GetValue();
      break;
    }
  }
  
  REQUIRE(val == "value");

  REQUIRE( lexer.Set("pascal", stc));
  REQUIRE( lexer.GetDisplayLexer() == "pascal");
  REQUIRE( lexer.GetScintillaLexer() == "pascal");
  REQUIRE(!lexer.CommentComplete("(*test").empty());
  REQUIRE( lexer.CommentComplete("(*test").EndsWith("     *)"));
  
  wxExLexer lexer2(wxExLexers::Get()->FindByText("// this is a cpp comment text"));
  REQUIRE( lexer2.IsOk());
  REQUIRE( lexer2.GetDisplayLexer() == "cpp");
  REQUIRE( lexer2.GetScintillaLexer() == "cpp");
  REQUIRE( lexer2.Set(lexer, stc));
  REQUIRE( lexer2.GetDisplayLexer() == "pascal");
  REQUIRE( lexer2.GetScintillaLexer() == "pascal");
  REQUIRE(!lexer2.CommentComplete("(*test").empty());
  REQUIRE( lexer2.CommentComplete("(*test").EndsWith("     *)"));
  
  lexer.Reset(stc);
  REQUIRE( lexer.GetDisplayLexer().empty());
  REQUIRE( lexer.GetScintillaLexer().empty());
  
  REQUIRE( lexer.Set("xsl", stc));
  REQUIRE( lexer.GetLanguage() == "xml");
  
  lexer.Apply(stc);
}
