////////////////////////////////////////////////////////////////////////////////
// Name:      test-defs.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define EX_CALC(COMPONENT)                                                     \
  const std::vector<std::pair<std::string, int>> calcs{                        \
    {"", 0},      {"  ", 0},    {"1 + 1", 2},  {"5+5", 10},  {"1 * 1", 1},     \
    {"1 - 1", 0}, {"2 / 1", 2}, {"2 / 0", 0},  {"2 < 2", 8}, {"2 > 1", 1},     \
    {"2 | 1", 3}, {"2 & 1", 0}, {"~0", -1},    {"4 % 3", 1}, {".", 1},         \
    {"xxx", 0},   {"%s", 0},    {"%s/xx/", 0}, {"'a", 2},    {"'t", 2},        \
    {"'u", 3},    {"$", 4}};                                                   \
                                                                               \
  for (const auto& calc : calcs)                                               \
  {                                                                            \
    if (const auto& val(COMPONENT->calculator(calc.first)); val)               \
    {                                                                          \
      REQUIRE(*val == calc.second);                                            \
    }                                                                          \
  }
