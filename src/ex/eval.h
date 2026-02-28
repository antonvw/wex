////////////////////////////////////////////////////////////////////////////////
// Name:      eval.h
// Purpose:   Declaration of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <expected>

namespace wex
{
class evaluator_imp;
class ex;

/// This class offers an (ex) expression evaluator.
class evaluator
{
public:
  /// Constructor, specify the ex component, e.g. for line number (.).
  evaluator(const ex* ex);

  /// Destructor.
  ~evaluator();

  /// Returns calculated value.
  std::expected<int, std::string> eval(
    /// text containing the expression to be evaluated
    const std::string& text);

  /// Returns token calculation, is called from eval.
  /// Returns 0 if error.
  int eval_token(
    /// token to be evaluated
    const std::string& token,
    /// number of be added or substracted
    int rhs = 0) const;

private:
  evaluator_imp* m_eval;
  const wex::ex* m_ex{nullptr};
};
}; // namespace wex
