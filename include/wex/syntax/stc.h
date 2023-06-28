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
  /// Default constructor.
  stc(const data::window& data = data::window())
    : factory::stc(data)
  {
    ;
  };

  /// Virtual interface

  virtual bool lexer_is_previewable() const final
  {
    return m_lexer.is_previewable();
  }

  virtual std::string lexer_name() const final
  {
    return m_lexer.display_lexer();
  }

  /// Other methods.

  /// Blames margin.
  void blame_margin(const blame* blame);

  /// Enables or disables folding depending on fold property
  /// (default not implemented).
  void fold(
    /// if document contains more than 'Auto fold' lines,
    /// or if fold_all (and fold property is on) is specified,
    /// always all lines are folded.
    bool fold_all = false);

  /// Returns the lexer.
  const auto& get_lexer() const { return m_lexer; }

  /// Returns the lexer.
  auto& get_lexer() { return m_lexer; }

  /// Returns renamed revision.
  std::string margin_get_revision_renamed() const;

  /// Sets an indicator at specified start and end pos.
  /// Returns false if indicator is not loaded.
  bool set_indicator(
    /// indicator to use
    const indicator& indicator,
    /// start pos, if -1 GetTargetStart is used
    int start = -1,
    /// end pos, if -1 GetTargetEnd is used
    int end = -1);

private:
  void fold_all();

  lexer m_lexer;
};
}; // namespace syntax
}; // namespace wex
