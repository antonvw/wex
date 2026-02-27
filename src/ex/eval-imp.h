////////////////////////////////////////////////////////////////////////////////
// Name:      eval-imp.h
// Purpose:   Declaration of class wex::evaluator_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <expected>

#include <wex/ex/ex.h>

namespace wex
{
class evaluator_imp
{
public:
  std::expected<int, std::string>
  eval(const wex::ex* ex, const std::string& text);
};
}; // namespace wex
