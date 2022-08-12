////////////////////////////////////////////////////////////////////////////////
// Name:      test.cpp
// Purpose:   Implementation of general test functions.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex/ex.h>
#include <wex/ex/macros.h>

#include "test.h"

std::vector<std::string> get_builtin_variables()
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
