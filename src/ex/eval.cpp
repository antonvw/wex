////////////////////////////////////////////////////////////////////////////////
// Name:      eval.cpp
// Purpose:   Implementation of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>
#include <wex/factory/stc.h>

#include "eval-imp.h"
#include "eval.h"

wex::evaluator::evaluator(const ex* ex)
  : m_eval(new wex::evaluator_imp)
  , m_ex(ex)
{
}

wex::evaluator::~evaluator()
{
  delete m_eval;
}

std::expected<int, std::string> wex::evaluator::eval(const std::string& text)
{
  if (text == "-")
  {
    return {m_ex->get_command().get_stc()->get_current_line()};
  }

  if (text == "+")
  {
    return {m_ex->get_command().get_stc()->get_current_line() + 2};
  }

  std::string expanded(text);
  marker_and_register_expansion(m_ex, expanded);
  return m_eval->eval(this, expanded);
}

int wex::evaluator::eval_token(const std::string& token, int rhs) const
{
  if (token.empty())
  {
    return 0;
  }

  switch (token[0])
  {
    case '.':
      return m_ex->get_command().get_stc()->get_current_line() + 1;

    case '$':
      if (m_ex->visual() == wex::ex::mode_t::EX)
      {
        return m_ex->ex_stream()->get_line_count_request();
      }
      return m_ex->get_command().get_stc()->get_line_count_request();

    case '\'':
      if (token.size() == 2)
      {
        return m_ex->marker_line(token[1]) + 1;
      }
      break;

    // A <plus-sign> ( '+' ) or a <hyphen-minus> ( '-' ) followed by
    // a decimal number shall address the current line plus or minus the number.
    // A '+' or '-' not followed by a decimal number shall address the current
    // line plus or minus 1.
    case '-':
      return m_ex->get_command().get_stc()->get_current_line() + 1 - rhs;

    case '+':
      return m_ex->get_command().get_stc()->get_current_line() + 1 + rhs;

    default:
      log("unhandled token") << token[0];
  }

  return 0;
}
