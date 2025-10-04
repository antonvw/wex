////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex.h>
#include <wex/ex/macros.h>

#include "test.h"

std::vector<std::pair<std::string, std::string>> wex::test::get_abbreviations()
{
  return std::vector<std::pair<std::string, std::string>>{
    {"XX", "GREAT"}, // see also test-source.txt
    {"YY", "WHITE"},
    {"ZZ", "SHARK"}};
}

std::vector<std::string> wex::test::get_builtin_variables()
{
  std::vector<std::string> v;

  for (const auto i : wex::ex::get_macros().get_variables())
  {
    if (i.second.is_builtin())
    {
      v.push_back(i.first);
    }
  }

  return v;
}

bool wex::test::ex::OnInit()
{
  if (!test::app::OnInit())
  {
    return false;
  }

  m_frame = new wex::frame();
  SetTopWindow(m_frame);
  m_stc = new test::stc();
  m_frame->Show();
  m_frame->pane_add(m_stc);

  return true;
}

wex::frame* frame()
{
  return wex::test::ex::frame();
}

wex::syntax::stc* get_stc()
{
  return wex::test::ex::get_stc();
}
