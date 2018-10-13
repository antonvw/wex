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
#include <wx/extension/lexer-props.h>

wex::lexer_props::lexer_props()
  : lexer("props")
{
}
  
const std::string wex::lexer_props::MakeComment(const std::string& comment) const
{
  return !comment.empty() ? 
    lexer::MakeComment(comment) + "\n": std::string();
}
  
const std::string wex::lexer_props::MakeKey(
  const std::string& name, 
  const std::string& value,
  const std::string& comment) const
{
  return 
    MakeComment(comment) +
    name + "=" + value + "\n";
}

const std::string wex::lexer_props::MakeSection(const std::string& section) const
{
  return "[" + section + "]\n";
}
