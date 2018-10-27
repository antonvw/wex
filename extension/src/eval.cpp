////////////////////////////////////////////////////////////////////////////////
// Name:      eval.cpp
// Purpose:   Implementation of class wex::evaluator
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <eval.hpp>
#include <wx/numformatter.h>
#include <wex/ex.h>
#include <wex/lexer-props.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vi-macros.h>
#include "eval.h"

wex::evaluator::~evaluator()
{
  delete m_eval;
}

std::tuple<double, int, std::string> wex::evaluator::Eval(
  wex::ex* ex, const std::string& text)
{
  Init();

  std::string expr(skip_white_space(text));

  if (expr.empty() || expr.find("%s") != std::string::npos)
  {
    return {0, 0, std::string()};
  }
  
  const char ds(wxNumberFormatter::GetDecimalSeparator());
  
  // Determine the width.
  const std::string rt((ds == '.' ? "\\.": std::string(1, ds)) + std::string("[0-9]+"));
  std::regex re(rt);
  
  int width = 0;

  if (const auto words_begin = std::sregex_iterator(text.begin(), text.end(), re),
    words_end = std::sregex_iterator();  
    words_begin != words_end)
  {
    if (std::smatch match = *words_begin; !match.empty())
    {
      width = match.length() - 1;
    }
  }
  else
  {
    // Replace . with current line.
    replace_all(expr, ".", std::to_string(
      ex->GetCommand().STC()->GetCurrentLine() + 1));
  }
  
  // Replace $ with line count.
  replace_all(expr, "$", 
    std::to_string(ex->GetCommand().STC()->GetLineCount()));
  
  // Expand all markers and registers.
  if (!marker_and_register_expansion(ex, expr))
  {
    return {0, 0, "marker or register error"};
  }

  // https://github.com/r-lyeh/eval
  std::string err;
  return {m_eval->eval(expr, &err), width, err};
}

std::string wex::evaluator::GetInfo(const wex::ex* ex)
{
  Init();
  
  const lexer_props l;
  std::string output(l.MakeSection("Named buffers"));

  for (const auto& it : ex->GetMacros().GetRegisters())
  {
    output += it;
  }

  output += l.MakeSection("Filename buffer");
  output += l.MakeKey("%", ex->GetCommand().STC()->GetFileName().GetFullName());

  if (!m_eval->variables.empty()) 
  {
    output += l.MakeSection("Variables");

    for (const auto &var : m_eval->variables) 
    {
      output += l.MakeKey(var, std::to_string(m_eval->eval(var, nullptr)));
    }
  }

  return output;
}

void wex::evaluator::Init()
{
  if (m_Initialized) return;

  try
  {
    m_eval = new evaluator_extra;

    // prevent a comma to be used as argument separator
    // for functions
    m_eval->opers.insert({",", ::evaluator::oper_t{false, 1, false}});
    m_eval->opers.insert({">>", ::evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"<<", ::evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"&", ::evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"|", ::evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"xor", ::evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"bitor", ::evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"bitand", ::evaluator::oper_t{false, 10, false}});
    
    m_eval->funcs.insert({",", m_eval->func_args(2, [](::evaluator::args_t v) {
      return v[0] + v[1] / 10;})});
    m_eval->funcs.insert({">>", m_eval->func_args(2, [](::evaluator::args_t v) {
      return (int)v[0] >> (int)v[1];})});
    m_eval->funcs.insert({"<<", m_eval->func_args(2, [](::evaluator::args_t v) {
      return (int)v[0] << (int)v[1];})});
    m_eval->funcs.insert({"&", m_eval->func_args(2, [](::evaluator::args_t v) {
      return (int)v[0] & (int)v[1];})});
    m_eval->funcs.insert({"|", m_eval->func_args(2, [](::evaluator::args_t v) {
      return (int)v[0] | (int)v[1];})});
    m_eval->funcs.insert({"compl", m_eval->func_args(1, [](::evaluator::args_t v) {
      return ~(int)v[0];})});
    m_eval->funcs.insert({"xor", m_eval->func_args(2, [](::evaluator::args_t v) {
      if (v.size() < 2) return 0;
      return (int)v[0] ^ (int)v[1];})});
    m_eval->funcs.insert({"bitor", m_eval->func_args(2, [](::evaluator::args_t v) {
      return (int)v[0] | (int)v[1];})});
    m_eval->funcs.insert({"bitand", m_eval->func_args(2, [](::evaluator::args_t v) {
      return (int)v[0] & (int)v[1];})});

    m_Initialized = true;
  }
  catch (std::exception& e)
  {
    log(e) << "evaluator";
  }
}
