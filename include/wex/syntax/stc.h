////////////////////////////////////////////////////////////////////////////////
// Name:      stc.h
// Purpose:   Declaration of class wex::syntax::stc
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/stc.h>
#include <wex/syntax/lexer.h>

namespace wex
{
namespace syntax
{

/// Offers a factory stc with lexer support (syntax colouring, folding).
class stc : public factory::stc
{
public:
  /// Virtual interface

  virtual std::string lexer_name() const final
  {
    return m_lexer.display_lexer();
  }

  virtual bool lexer_is_previewable() const final
  {
    return m_lexer.is_previewable();
  }

  /// Other methods.

  /// Returns the lexer.
  const auto& get_lexer() const { return m_lexer; }

  /// Returns the lexer.
  auto& get_lexer() { return m_lexer; }

private:
  lexer m_lexer;
};
}; // namespace syntax
}; // namespace wex
