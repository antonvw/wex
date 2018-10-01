////////////////////////////////////////////////////////////////////////////////
// Name:      lexer-props.h
// Purpose:   Declaration of wxExLexerProps class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/lexer.h>

/// This class defines a lexer properties class.
class wxExLexerProps : public wxExLexer
{
public:
  /// Default constructor.
  wxExLexerProps();

  /// Returns a comment string.
  const std::string MakeComment(const std::string& comment) const;
  
  /// Returns a key value string with optional comment.
  const std::string MakeKey(
    const std::string& name, 
    const std::string& value,
    const std::string& comment = std::string()) const;
  
  /// Returns a section string.
  const std::string MakeSection(const std::string& section) const;
};
