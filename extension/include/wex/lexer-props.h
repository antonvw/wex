////////////////////////////////////////////////////////////////////////////////
// Name:      lexer-props.h
// Purpose:   Declaration of wex::lexer_props class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/lexer.h>

namespace wex
{
  /// This class defines a lexer properties class.
  class lexer_props : public lexer
  {
  public:
    /// Default constructor.
    lexer_props();

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
};
