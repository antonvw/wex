////////////////////////////////////////////////////////////////////////////////
// Name:      eval.cpp
// Purpose:   Implementation of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <list>
#include <numeric>
#include <regex>

#include <boost/config/warning_disable.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/spirit/home/x3.hpp>
#include <boost/spirit/home/x3/support/ast/variant.hpp>

#include <wex/ex-stream.h>
#include <wex/ex.h>
#include <wex/factory/stc.h>

#include "eval.h"

namespace x3 = boost::spirit::x3;

namespace wex
{
namespace ast
{
struct ex;
struct nil
{
};
struct program;
struct signed_;

struct operand
  : x3::variant<
      nil,
      unsigned int,
      x3::forward_ast<ex>,
      x3::forward_ast<signed_>,
      x3::forward_ast<program>>
{
  using base_type::base_type;
  using base_type::operator=;
};

struct ex
{
  std::string token;
};

struct signed_
{
  char    sign;
  operand operand_;
};

struct operation
{
  char    operator_;
  operand operand_;
};

struct program
{
  operand              first;
  std::list<operation> rest;
};

struct eval
{
  explicit eval(const wex::ex* ex)
    : m_ex(ex)
  {
    ;
  };

  auto operator()(nil) const
  {
    BOOST_ASSERT(0);
    return 0;
  }

  auto operator()(ex const& e) const
  {
    if (e.token == ".")
    {
      return m_ex->get_command().get_stc()->get_current_line() + 1;
    }
    else if (e.token == "$")
    {
      if (m_ex->visual() == wex::ex::EX)
      {
        return m_ex->ex_stream()->get_line_count_request();
      }
      else
      {
        return m_ex->get_command().get_stc()->get_line_count_request();
      }
    }
    else if (e.token[0] == '\'' && e.token.size() == 2)
    {
      const int line = m_ex->marker_line(e.token[1]);
      if (line == -1)
        throw std::overflow_error("no marker");
      return line + 1;
    }

    BOOST_ASSERT(0);
    return 0;
  }

  auto operator()(int n) const { return n; }

  int operator()(int lhs, operation const& x) const
  {
    switch (const auto rhs = boost::apply_visitor(*this, x.operand_);
            x.operator_)
    {
      case '+':
        return lhs + rhs;
      case '-':
        return lhs - rhs;
      case '*':
        return lhs * rhs;
      case '%':
        return lhs % rhs;
      case '&':
        return lhs & rhs;
      case '|':
        return lhs | rhs;
      case '^':
        return lhs ^ rhs;
      case '<':
        return lhs << rhs;
      case '>':
        return lhs >> rhs;
      case '/':
        if (rhs == 0)
          throw std::overflow_error("divide by zero");
        return lhs / rhs;
    }
    BOOST_ASSERT(0);
    return 0;
  }

  int operator()(signed_ const& x) const
  {
    int rhs = boost::apply_visitor(*this, x.operand_);
    switch (x.sign)
    {
      case '~':
        return ~rhs;
      case '-':
        return -rhs;
      case '+':
        return +rhs;
    }
    BOOST_ASSERT(0);
    return 0;
  }

  int operator()(program const& x) const
  {
    return std::accumulate(
      x.rest.begin(),
      x.rest.end(),
      boost::apply_visitor(*this, x.first),
      *this);
  }

  const wex::ex* m_ex;
};
} // namespace ast

namespace calculator_grammar
{
using x3::alnum;
using x3::char_;
using x3::string;
using x3::uint_;

x3::rule<class expression, ast::program> const expression("expression");
x3::rule<class term, ast::program> const       term("term");
x3::rule<class factor, ast::operand> const     factor("factor");
x3::rule<class identifier, ast::ex> const      identifier("identifier");

auto const expression_def = term >>
                            *((char_('+') >> term) | (char_('-') >> term));

auto const term_def = factor >>
                      *((char_('*') >> factor) | (char_('/') >> factor) |
                        (char_('%') >> factor) | (char_('&') >> factor) |
                        (char_('|') >> factor) | (char_('^') >> factor) |
                        (char_('<') >> factor) | (char_('>') >> factor));

auto const factor_def = uint_ | identifier | '(' >> expression >> ')' |
                        (char_('~') >> factor) | (char_('-') >> factor) |
                        (char_('+') >> factor);

auto const identifier_def = string(".") | string("$") |
                            (string("'") >> (alnum | char_('<') | char_('>')));

BOOST_SPIRIT_DEFINE(expression, term, factor, identifier);

auto calculator = expression;
} // namespace calculator_grammar

using calculator_grammar::calculator;

class evaluator_imp
{
public:
  std::tuple<int, std::string> eval(const wex::ex* ex, const std::string& text)
  {
    try
    {
      auto&        calc = wex::calculator;
      ast::program program;
      ast::eval    eval(ex);

      auto                                 iter = text.begin();
      boost::spirit::x3::ascii::space_type space;
      const bool r = phrase_parse(iter, text.end(), calc, space, program);

      if (r && iter == text.end())
      {
        return {eval(program), std::string()};
      }
      else
      {
        return {0, std::string(iter, text.end())};
      }
    }
    catch (std::exception& e)
    {
      return {0, e.what()};
    }
  };
};
}; // namespace wex

BOOST_FUSION_ADAPT_STRUCT(wex::ast::ex, (std::string, token))
BOOST_FUSION_ADAPT_STRUCT(wex::ast::operation, (char, operator_), operand_)
BOOST_FUSION_ADAPT_STRUCT(wex::ast::program, first, rest)
BOOST_FUSION_ADAPT_STRUCT(wex::ast::signed_, (char, sign), operand_)

wex::evaluator::evaluator()
  : m_eval(new wex::evaluator_imp)
{
}

wex::evaluator::~evaluator()
{
  delete m_eval;
}

std::tuple<int, std::string>
wex::evaluator::eval(const wex::ex* ex, const std::string& text) const
{
  std::string expanded(text);
  marker_and_register_expansion(ex, expanded);
  return m_eval->eval(ex, expanded);
}
