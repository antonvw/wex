////////////////////////////////////////////////////////////////////////////////
// Name:      eval.h
// Purpose:   Declaration of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>

namespace wex
{
class evaluator_imp;
class ex;

/// This class offers an (ex) expression evaluator.
class evaluator
{
public:
  /// If there was an error during last eval.
  static std::string error() { return m_error; };

  /// Default constructor.
  evaluator();

  /// Destructor.
  ~evaluator();

  /// Returns calculated value.
  std::optional<int> eval(
    /// the ex component, e.g. for line number (.) if present in text
    const ex* ex,
    /// text containing the expression to be evaluated
    const std::string& text) const;

private:
  evaluator_imp*            m_eval;
  static inline std::string m_error;
};
}; // namespace wex
