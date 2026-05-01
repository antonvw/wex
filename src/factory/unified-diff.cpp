////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.cpp
// Purpose:   Implementation of class wex::factory::unified_diff
//            https://www.gnu.org/software/diffutils/manual/html_node/Detailed-Unified.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/factory/frame.h>
#include <wex/factory/unified-diff.h>
#include <wx/app.h>

#include <algorithm>
#include <utility>

#include "unified-diff-parser.h"

wex::factory::unified_diff::unified_diff(
  std::string     input,
  factory::frame* frame)
  : m_input(std::move(input))
  , m_range{0, 0, 0, 0}
  , m_frame(frame)
{
  if (
    auto* frame = dynamic_cast<wex::factory::frame*>(wxTheApp->GetTopWindow());
    frame != nullptr && m_frame != nullptr)
  {
    m_frame = frame;
  }

  if (m_frame != nullptr)
  {
    m_frame->page_save();
  }
}

bool wex::factory::unified_diff::parse()
{
  return unified_diff_parser(this).parse();
}

bool wex::factory::unified_diff::report_diff()
{
  return m_frame != nullptr && m_frame->report_unified_diff(this);
}

void wex::factory::unified_diff::report_diff_finish()
{
  if (m_frame != nullptr)
  {
    m_frame->report_unified_diff(this);
    m_frame->page_restore();
  }
}

wex::path wex::factory::unified_diff::report_path() const
{
  return path_to();
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
