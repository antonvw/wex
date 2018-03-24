////////////////////////////////////////////////////////////////////////////////
// Name:      eval.h
// Purpose:   Declaration of class wxExEvaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

class evaluator_extra;
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
  double Eval(
    /// the ex component, e.g. for line number (.) if present in text
    wxExEx* ex, 
    /// text containing the expression to be evaluated
    const std::string& text, 
    /// the width to present the number
    int& width, 
    /// possible error during evaluation
    std::string& err);

  /// Returns the info for variables.
  std::string GetInfo(const wxExEx* ex);
private:
  void Init();

  bool m_Initialized {false};
  evaluator_extra* m_eval {nullptr};
};
