////////////////////////////////////////////////////////////////////////////////
// Name:      test-tokenizer.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <vector>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/tokenizer.h>
#include "../test.h"

TEST_CASE( "wex::tokenizer" ) 
{
  REQUIRE( wex::tokenizer("one two three four").GetString() == "one two three four");
  REQUIRE( wex::tokenizer("one two three four").GetToken().empty());
  REQUIRE( wex::tokenizer("one two three four").GetNextToken() == "one");
  
  wex::tokenizer tkz("one   two 			three four");
  REQUIRE( tkz.HasMoreTokens());
  REQUIRE( tkz.GetNextToken() == "one");
  REQUIRE( tkz.GetNextToken() == "two");
  REQUIRE( tkz.GetNextToken() == "three");
  REQUIRE( tkz.GetString() == "four");
  REQUIRE( tkz.GetLastDelimiter() == ' ');
  REQUIRE( tkz.GetNextToken() == "four");
  REQUIRE( tkz.GetToken() == "four");
  REQUIRE( tkz.GetLastDelimiter() == 0);
  REQUIRE( tkz.GetString().empty());
  REQUIRE( tkz.GetNextToken().empty());
  REQUIRE(!tkz.HasMoreTokens());
  
  wex::tokenizer tkz2("*.txt", ";");
  REQUIRE( tkz2.GetNextToken() == "*.txt");
  
  const auto l(tkz.Tokenize<std::list<std::string>>());
  const auto v(tkz.Tokenize<std::vector<std::string>>());
  REQUIRE( l.size() == 4);
  REQUIRE( v.size() == 4);
  
  REQUIRE( wex::tokenizer("one two three four").CountTokens() == 4);
  REQUIRE( wex::tokenizer("one;two;three;four").CountTokens() == 1);
  REQUIRE( wex::tokenizer("one two three;four", "; ").CountTokens() == 4);
  REQUIRE( wex::tokenizer("one two three four", "").CountTokens() == 0);

  REQUIRE( wex::tokenizer(";one;two;three;four", "; ").CountTokens() == 4);
  REQUIRE( wex::tokenizer(";;;one;two;three;four", "; ", false).CountTokens() == 7);

  REQUIRE( wex::tokenizer(" 1 2 3 4 5").Tokenize().size() == 5);
}
