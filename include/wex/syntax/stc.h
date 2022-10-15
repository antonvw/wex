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
class blame;
class indicator;

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

  /// Blames margin.
  void blame_margin(const blame* blame);

  /// Returns the lexer.
  const auto& get_lexer() const { return m_lexer; }

  /// Returns the lexer.
  auto& get_lexer() { return m_lexer; }

  /// Returns renamed revision.
  std::string margin_get_revision_renamed() const;

  /// Sets an indicator at specified start and end pos.
  /// Default false, not implemented.
  bool set_indicator(
    /// indicator to use
    const indicator& indicator,
    /// start pos, if -1 GetTargetStart is used
    int start = -1,
    /// end pos, if -1 GetTargetEnd is used
    int end = -1);

private:
  lexer m_lexer;
};
}; // namespace syntax
}; // namespace wex
