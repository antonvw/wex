////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.cpp
// Purpose:   Implementation of wxExLexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/lexer-props.h>

wxExLexerProps::wxExLexerProps()
  : wxExLexer("props")
{
}
  
const std::string wxExLexerProps::MakeComment(const std::string& comment) const
{
  return !comment.empty() ? 
    wxExLexer::MakeComment(comment) + "\n": std::string();
}
  
const std::string wxExLexerProps::MakeKey(
  const std::string& name, 
  const std::string& value,
  const std::string& comment) const
{
  return 
    MakeComment(comment) +
    name + "=" + value + "\n";
}

const std::string wxExLexerProps::MakeSection(const std::string& section) const
{
  return "[" + section + "]\n";
}
