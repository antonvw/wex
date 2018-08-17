////////////////////////////////////////////////////////////////////////////////
// Name:      eval.h
// Purpose:   Declaration of class wxExEvaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>

struct evaluator_extra;
class wxExEx;

/// This class offers methods to evaluate expressions.
class wxExEvaluator
{
public:
  /// Default constructor.
  wxExEvaluator() {;};

  /// Destructor.
 ~wxExEvaluator();

  /// Runs a calculation.
  /// Returns calculated value with precision width of text, and possible error.
  std::tuple<double, int, std::string> Eval(
    /// the ex component, e.g. for line number (.) if present in text
    wxExEx* ex, 
    /// text containing the expression to be evaluated
    const std::string& text);

  /// Returns the info for variables.
  std::string GetInfo(const wxExEx* ex);
private:
  void Init();

  bool m_Initialized {false};
  evaluator_extra* m_eval {nullptr};
};
