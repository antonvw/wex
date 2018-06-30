////////////////////////////////////////////////////////////////////////////////
// Name:      eval.cpp
// Purpose:   Implementation of class wxExEvaluator
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
#include <wx/extension/ex.h>
#include <wx/extension/log.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vi-macros.h>
#include "eval.h"

wxExEvaluator::~wxExEvaluator()
{
  delete m_eval;
}

std::tuple<double, int, std::string> wxExEvaluator::Eval(wxExEx* ex, const std::string& text)
{
  Init();

  std::string expr(wxExSkipWhiteSpace(text));

  if (expr.empty() || expr.find("%s") != std::string::npos)
  {
    return {0, 0, "empty expression"};
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
    wxExReplaceAll(expr, ".", std::to_string(
      ex->GetCommand().STC()->GetCurrentLine() + 1));
  }
  
  // Replace $ with line count.
  wxExReplaceAll(expr, "$", 
    std::to_string(ex->GetCommand().STC()->GetLineCount()));
  
  // Expand all markers and registers.
  if (!wxExMarkerAndRegisterExpansion(ex, expr))
  {
    return {0, 0, "marker or register error"};
  }

  // https://github.com/r-lyeh/eval
  std::string err;
  return {m_eval->eval(expr, &err), width, err};
}

std::string wxExEvaluator::GetInfo(const wxExEx* ex)
{
  Init();

  std::string output("[Named buffers]\n");

  for (const auto& it : ex->GetMacros().GetRegisters())
  {
    output += it + "\n";
  }

  output += "[Filename buffer]\n";
  output += "%: " + ex->GetCommand().STC()->GetFileName().GetFullName() + "\n";

  if (!m_eval->variables.empty()) 
  {
    output += "[Variables]\n";

    for (const auto &var : m_eval->variables) 
    {
      output += var + "=" + std::to_string(m_eval->eval(var, nullptr)) + "\n";
    }
  }

  return output;
}

void wxExEvaluator::Init()
{
  if (m_Initialized) return;

  try
  {
    m_eval = new evaluator_extra;

    // prevent a comma to be used as argument separator
    // for functions
    m_eval->opers.insert({",", evaluator::oper_t{false, 1, false}});
    m_eval->opers.insert({">>", evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"<<", evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"&", evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"|", evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"xor", evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"bitor", evaluator::oper_t{false, 10, false}});
    m_eval->opers.insert({"bitand", evaluator::oper_t{false, 10, false}});
    
    m_eval->funcs.insert({",", m_eval->func_args(2, [](evaluator::args_t v) {
      return v[0] + v[1] / 10;})});
    m_eval->funcs.insert({">>", m_eval->func_args(2, [](evaluator::args_t v) {
      return (int)v[0] >> (int)v[1];})});
    m_eval->funcs.insert({"<<", m_eval->func_args(2, [](evaluator::args_t v) {
      return (int)v[0] << (int)v[1];})});
    m_eval->funcs.insert({"&", m_eval->func_args(2, [](evaluator::args_t v) {
      return (int)v[0] & (int)v[1];})});
    m_eval->funcs.insert({"|", m_eval->func_args(2, [](evaluator::args_t v) {
      return (int)v[0] | (int)v[1];})});
    m_eval->funcs.insert({"compl", m_eval->func_args(1, [](evaluator::args_t v) {
      return ~(int)v[0];})});
    m_eval->funcs.insert({"xor", m_eval->func_args(2, [](evaluator::args_t v) {
      if (v.size() < 2) return 0;
      return (int)v[0] ^ (int)v[1];})});
    m_eval->funcs.insert({"bitor", m_eval->func_args(2, [](evaluator::args_t v) {
      return (int)v[0] | (int)v[1];})});
    m_eval->funcs.insert({"bitand", m_eval->func_args(2, [](evaluator::args_t v) {
      return (int)v[0] & (int)v[1];})});

    m_Initialized = true;
  }
  catch (std::exception& e)
  {
    wxExLog(e) << "evaluator";
  }
}
