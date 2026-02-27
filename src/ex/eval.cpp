////////////////////////////////////////////////////////////////////////////////
// Name:      eval.cpp
// Purpose:   Implementation of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex.h>
#include <wex/factory/stc.h>

#include "eval-imp.h"
#include "eval.h"

wex::evaluator::evaluator()
  : m_eval(new wex::evaluator_imp)
{
}

wex::evaluator::~evaluator()
{
  delete m_eval;
}

std::expected<int, std::string>
wex::evaluator::eval(const wex::ex* ex, const std::string& text) const
{
  if (text == "-")
  {
    return {ex->get_command().get_stc()->get_current_line()};
  }

  if (text == "+")
  {
    return {ex->get_command().get_stc()->get_current_line() + 2};
  }

  std::string expanded(text);
  marker_and_register_expansion(ex, expanded);
  return m_eval->eval(ex, expanded);
}
