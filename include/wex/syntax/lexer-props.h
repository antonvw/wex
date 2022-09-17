////////////////////////////////////////////////////////////////////////////////
// Name:      lexer-props.h
// Purpose:   Declaration of wex::lexer_props class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/lexer.h>

namespace wex
{
/// This class defines a lexer properties class.
class lexer_props : public lexer
{
public:
  /// Default constructor.
  lexer_props();

  /// Returns a comment string.
  const std::string make_comment(const std::string& comment) const;

  /// Returns a key value string with optional comment.
  const std::string make_key(
    const std::string& name,
    const std::string& value,
    const std::string& comment = std::string()) const;

  /// Returns a section string.
  const std::string make_section(const std::string& section) const;
};
}; // namespace wex
