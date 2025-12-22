////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.cpp
// Purpose:   Implementation of class wex::factory::unified_diff
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/factory/unified-diff.h>

#include <algorithm>
#include <utility>

#include "unified-diff-parser.h"

wex::factory::unified_diff::unified_diff(std::string input)
  : m_input(std::move(input))
{
}

bool wex::factory::unified_diff::parse()
{
  return unified_diff_parser(this).parse();
}

void wex::factory::unified_diff::trace(const std::string& text) const
{
  if (log::get_level() != log::level_t::TRACE)
  {
    return;
  }

  using boost::describe::operators::operator<<;

  std::stringstream str;
  str << "type: " << boost::describe::enum_to_string(m_type, "none") << " "
      << *this << " ranges: ";

  std::ranges::for_each(
    m_range,
    [this, &str](const auto& it)
    {
      str << std::to_string(it) << ",";
    });

  str << " text sizes: ";

  std::ranges::for_each(
    m_text,
    [this, &str](const auto& it)
    {
      str << it.size() << ",";
    });

  str << " paths: ";

  std::ranges::for_each(
    m_path,
    [this, &str](const auto& it)
    {
      str << it.string() << ",";
    });

  log::trace("unified_diff::" + text) << str.str();
}
