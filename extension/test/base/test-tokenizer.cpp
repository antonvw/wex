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
#include <wex/tokenizer.h>
#include "../test.h"

TEST_CASE( "wex::tokenizer" ) 
{
  REQUIRE( wex::tokenizer("one two three four").get_string() == "one two three four");
  REQUIRE( wex::tokenizer("one two three four").get_token().empty());
  REQUIRE( wex::tokenizer("one two three four").get_next_token() == "one");
  
  wex::tokenizer tkz("one   two 			three four");
  REQUIRE( tkz.has_more_tokens());
  REQUIRE( tkz.get_next_token() == "one");
  REQUIRE( tkz.get_next_token() == "two");
  REQUIRE( tkz.get_next_token() == "three");
  REQUIRE( tkz.get_string() == "four");
  REQUIRE( tkz.last_delimiter() == ' ');
  REQUIRE( tkz.get_next_token() == "four");
  REQUIRE( tkz.get_token() == "four");
  REQUIRE( tkz.last_delimiter() == 0);
  REQUIRE( tkz.get_string().empty());
  REQUIRE( tkz.get_next_token().empty());
  REQUIRE(!tkz.has_more_tokens());
  
  wex::tokenizer tkz2("*.txt", ";");
  REQUIRE( tkz2.get_next_token() == "*.txt");
  
  const auto l(tkz.tokenize<std::list<std::string>>());
  const auto v(tkz.tokenize<std::vector<std::string>>());
  REQUIRE( l.size() == 4);
  REQUIRE( v.size() == 4);
  
  REQUIRE( wex::tokenizer("one two three four").count_tokens() == 4);
  REQUIRE( wex::tokenizer("one;two;three;four").count_tokens() == 1);
  REQUIRE( wex::tokenizer("one two three;four", "; ").count_tokens() == 4);
  REQUIRE( wex::tokenizer("one two three four", "").count_tokens() == 0);

  REQUIRE( wex::tokenizer(";one;two;three;four", "; ").count_tokens() == 4);
  REQUIRE( wex::tokenizer(";;;one;two;three;four", "; ", false).count_tokens() == 7);

  REQUIRE( wex::tokenizer(" 1 2 3 4 5").tokenize().size() == 5);
}
