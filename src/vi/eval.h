////////////////////////////////////////////////////////////////////////////////
// Name:      eval.h
// Purpose:   Declaration of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

import<tuple>;

namespace wex
{
class evaluator_imp;
class ex;

/// This class offers an (ex) expression evaluator.
class evaluator
{
public:
  /// Default constructor.
  evaluator();

  /// Destructor.
  ~evaluator();

  /// Returns calculated value and possible error.
  std::tuple<int, std::string> eval(
    /// the ex component, e.g. for line number (.) if present in text
    const ex* ex,
    /// text containing the expression to be evaluated
    const std::string& text) const;

private:
  evaluator_imp* m_eval;
};
}; // namespace wex
