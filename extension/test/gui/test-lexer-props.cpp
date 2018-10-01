////////////////////////////////////////////////////////////////////////////////
// Name:      test-lexer-props.cpp
// Purpose:   Implementation for wxExtension unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/lexer-props.h>
#include "test.h"

TEST_CASE("wxExLexerProps")
{
  const wxExLexerProps l;
  
  REQUIRE( l.MakeComment("").find(';') == std::string::npos);
  REQUIRE( l.MakeComment("test").find(';') != std::string::npos);
  
  REQUIRE( l.MakeKey("xx", "yy").find('=') != std::string::npos);
  REQUIRE( l.MakeKey("xx", "yy", "zz").find('=') != std::string::npos);
  REQUIRE( l.MakeKey("xx", "yy", "zz").find(';') != std::string::npos);
  
  REQUIRE( l.MakeSection("test").find('[') != std::string::npos);
}
