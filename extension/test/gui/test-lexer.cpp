////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer.cpp
// Purpose:   Implementation for wxExtension cpp unit testing
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

void fixture::testLexer()
{
  wxExSTC* stc = new wxExSTC(m_Frame, "hello stc");
  
  wxExLexer lexer;
  CPPUNIT_ASSERT(!lexer.IsOk());
  CPPUNIT_ASSERT( lexer.GetStyles().empty());
  
  CPPUNIT_ASSERT( wxExLexer("cpp").IsOk());
  CPPUNIT_ASSERT( wxExLexer("pascal").IsOk());
  CPPUNIT_ASSERT(!wxExLexer("xxx").IsOk());
  
  lexer = wxExLexers::Get()->FindByText("XXXX");
  CPPUNIT_ASSERT(!lexer.IsOk());
  
  lexer = wxExLexers::Get()->FindByText("<html>");
  CPPUNIT_ASSERT( lexer.IsOk());
  CPPUNIT_ASSERT( lexer.GetDisplayLexer() == "hypertext");
  
  lexer = wxExLexers::Get()->FindByText("// this is a cpp comment text");
  CPPUNIT_ASSERT( lexer.IsOk());
  CPPUNIT_ASSERT( wxExLexer(lexer).IsOk());
  CPPUNIT_ASSERT( lexer.GetDisplayLexer() == "cpp");
  CPPUNIT_ASSERT( lexer.GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT( lexer.UsableCharactersPerLine() > 0);
  CPPUNIT_ASSERT(!lexer.GetExtensions().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentBegin2().empty());
  CPPUNIT_ASSERT( lexer.GetCommentEnd().empty());
  CPPUNIT_ASSERT(!lexer.GetCommentEnd2().empty());
  CPPUNIT_ASSERT( lexer.GetLanguage().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
  CPPUNIT_ASSERT(!lexer.GetStyles().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywordsString().empty());
  CPPUNIT_ASSERT(!lexer.GetKeywordsString(-1, 0).empty());
  CPPUNIT_ASSERT(!lexer.GetKeywordsString(-1, 6).empty());
  CPPUNIT_ASSERT( lexer.GetKeywordsString(-1, 8).Contains("for_each"));
  CPPUNIT_ASSERT(!lexer.GetKeywordsString(-1, 9).Contains("for_each"));
  CPPUNIT_ASSERT( lexer.GetKeywordsString(-1, 50).empty());
  CPPUNIT_ASSERT( lexer.CommentComplete("// test").empty());

  CPPUNIT_ASSERT( lexer.IsKeyword("class"));
  CPPUNIT_ASSERT( lexer.IsKeyword("const"));

  CPPUNIT_ASSERT( lexer.KeywordStartsWith("cla"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xxx"));

  CPPUNIT_ASSERT(!lexer.MakeComment("test", true).empty());
  CPPUNIT_ASSERT(!lexer.MakeComment("test", "test").empty());
  CPPUNIT_ASSERT(!lexer.MakeSingleLineComment("test").empty());

  CPPUNIT_ASSERT( lexer.GetKeywordsString(6).empty());
  CPPUNIT_ASSERT( lexer.AddKeywords("hello:1"));
  CPPUNIT_ASSERT( lexer.AddKeywords("more:1"));
  CPPUNIT_ASSERT( lexer.AddKeywords(
    "test11 test21:1 test31:1 test12:2 test22:2"));
  CPPUNIT_ASSERT( lexer.AddKeywords("final", 6));
  CPPUNIT_ASSERT(!lexer.AddKeywords(""));
  CPPUNIT_ASSERT(!lexer.AddKeywords("xxx:1", -1));
  CPPUNIT_ASSERT(!lexer.AddKeywords("xxx:1", 100));
  CPPUNIT_ASSERT(!lexer.GetKeywordsString(6).empty());

  CPPUNIT_ASSERT( lexer.IsKeyword("hello")); 
  CPPUNIT_ASSERT( lexer.IsKeyword("more")); 
  CPPUNIT_ASSERT( lexer.IsKeyword("class")); 
  CPPUNIT_ASSERT( lexer.IsKeyword("test11"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test21"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test12"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test22"));
  CPPUNIT_ASSERT( lexer.IsKeyword("test31"));
  CPPUNIT_ASSERT( lexer.IsKeyword("final"));
  CPPUNIT_ASSERT(!lexer.IsKeyword("xxx"));

  CPPUNIT_ASSERT( lexer.KeywordStartsWith("te"));
  CPPUNIT_ASSERT(!lexer.KeywordStartsWith("xx"));

  CPPUNIT_ASSERT(!lexer.GetKeywords().empty());
  
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
  
  CPPUNIT_ASSERT(val == "value");

  CPPUNIT_ASSERT( lexer.Set("pascal", stc));
  CPPUNIT_ASSERT( lexer.GetDisplayLexer() == "pascal");
  CPPUNIT_ASSERT( lexer.GetScintillaLexer() == "pascal");
  CPPUNIT_ASSERT(!lexer.CommentComplete("(*test").empty());
  CPPUNIT_ASSERT( lexer.CommentComplete("(*test").EndsWith("     *)"));
  
  wxExLexer lexer2(wxExLexers::Get()->FindByText("// this is a cpp comment text"));
  CPPUNIT_ASSERT( lexer2.IsOk());
  CPPUNIT_ASSERT( lexer2.GetDisplayLexer() == "cpp");
  CPPUNIT_ASSERT( lexer2.GetScintillaLexer() == "cpp");
  CPPUNIT_ASSERT( lexer2.Set(lexer, stc));
  CPPUNIT_ASSERT( lexer2.GetDisplayLexer() == "pascal");
  CPPUNIT_ASSERT( lexer2.GetScintillaLexer() == "pascal");
  CPPUNIT_ASSERT(!lexer2.CommentComplete("(*test").empty());
  CPPUNIT_ASSERT( lexer2.CommentComplete("(*test").EndsWith("     *)"));
  
  lexer.Reset(stc);
  CPPUNIT_ASSERT( lexer.GetDisplayLexer().empty());
  CPPUNIT_ASSERT( lexer.GetScintillaLexer().empty());
  
  CPPUNIT_ASSERT( lexer.Set("xsl", stc));
  CPPUNIT_ASSERT( lexer.GetLanguage() == "xml");
  
  lexer.Apply(stc);
}
