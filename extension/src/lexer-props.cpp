////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wex::lexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/lexer-props.h>

wex::lexer_props::lexer_props()
  : lexer("props")
{
}
  
const std::string wex::lexer_props::make_comment(const std::string& comment) const
{
  return !comment.empty() ? 
    lexer::make_comment(comment) + "\n": std::string();
}
  
const std::string wex::lexer_props::make_key(
  const std::string& name, 
  const std::string& value,
  const std::string& comment) const
{
  return 
    make_comment(comment) +
    name + "=" + value + "\n";
}

const std::string wex::lexer_props::make_section(const std::string& section) const
{
  return "[" + section + "]\n";
}
