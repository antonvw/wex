////////////////////////////////////////////////////////////////////////////////
// Name:      eval-imp.h
// Purpose:   Declaration of class wex::evaluator_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <expected>

namespace wex
{
class evaluator;

class evaluator_imp
{
public:
  std::expected<int, std::string>
  eval(const evaluator* ev, const std::string& text);
};
}; // namespace wex
