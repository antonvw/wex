////////////////////////////////////////////////////////////////////////////////
// Name:      eval.h
// Purpose:   Declaration of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <tuple>

struct evaluator_extra;

namespace wex
{
  class ex;

  /// This class offers methods to evaluate expressions.
  class evaluator
  {
  public:
    /// Default constructor.
    evaluator() {;};

    /// Destructor.
   ~evaluator();

    /// Runs a calculation.
    /// Returns calculated value with precision width of text, and possible error.
    std::tuple<double, int, std::string> Eval(
      /// the ex component, e.g. for line number (.) if present in text
      ex* ex, 
      /// text containing the expression to be evaluated
      const std::string& text);

    /// Returns the info for variables.
    std::string GetInfo(const ex* ex);
  private:
    void Init();

    bool m_Initialized {false};
    evaluator_extra* m_eval {nullptr};
  };
};
