////////////////////////////////////////////////////////////////////////////////
// Name:      blame.cpp
// Purpose:   Implementation of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/chrono.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/factory/blame.h>

namespace wex
{
std::string
build(const std::string& key, const std::string& field, bool first = false)
{
  std::string add;

  if (config("blame." + key).get(true))
  {
    if (!first)
    {
      add += " ";
    }

    add += field;
  }

  return add;
}
} // namespace wex

wex::blame::blame(const pugi::xml_node& node)
  : m_blame_format(node.attribute("blame-format").value())
  , m_date_format(node.attribute("date-format").value())
  , m_date_print(node.attribute("date-print").as_uint())
{
}

wex::lexers::margin_style_t wex::blame::get_style(const std::string& text) const
{
  lexers::margin_style_t style = lexers::margin_style_t::UNKNOWN;

  if (text.empty())
  {
    return style;
  }

  if (const auto& [r, t] = chrono(m_date_format).get_time(text); r)
  {
    const time_t now               = time(nullptr);
    const auto   dt                = difftime(now, t);
    const int    seconds           = 1;
    const int    seconds_in_minute = 60 * seconds;
    const int    seconds_in_hour   = 60 * seconds_in_minute;
    const int    seconds_in_day    = 24 * seconds_in_hour;
    const int    seconds_in_week   = 7 * seconds_in_day;
    const int    seconds_in_month  = 30 * seconds_in_day;
    const int    seconds_in_year   = 365 * seconds_in_day;

    if (dt < seconds_in_day)
    {
      style = lexers::margin_style_t::DAY;
    }
    else if (dt < seconds_in_week)
    {
      style = lexers::margin_style_t::WEEK;
    }
    else if (dt < seconds_in_month)
    {
      style = lexers::margin_style_t::MONTH;
    }
    else if (dt < seconds_in_year)
    {
      style = lexers::margin_style_t::YEAR;
    }
    else
    {
      style = lexers::margin_style_t::OTHER;
    }
  }

  return style;
}

bool wex::blame::parse(const std::string& text)
{
  try
  {
    if (regex r(m_blame_format); r.search(text) >= 3)
    {
      if (r.matches().size() == 3)
      {
        return parse_compact(text, r);
      }
      else if (r.matches().size() == 6)
      {
        return parse_full(text, r);
      }
      else
      {
        log("blame parsing") << r.matches().size();
        return false;
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << "blame:" << text;
  }

  return false;
}

// svn
// 0 -> id
// 1 -> author
// 2 -> date
bool wex::blame::parse_compact(const std::string& line, const regex& r)
{
  m_info = build("id", r[0], true) + build("author", r[1]) +
           build("date", r[2].substr(0, m_date_print));

  if (m_info.empty())
  {
    m_info = " ";
  }

  m_skip_info = false;
  m_style     = get_style(r[2]);

  return true;
}

// git
// 0 -> id
// 1 -> path, or empty
// 2 -> author
// 3 -> date
// 4 -> line no
// 5 -> original line text
bool wex::blame::parse_full(const std::string& line, const regex& r)
{
  m_info = build("id", r[0], true) + build("author", r[2]) +
           build("date", r[3].substr(0, m_date_print));

  if (m_info.empty())
  {
    m_info = " ";
  }

  m_skip_info = false;
  m_path      = boost::algorithm::trim_copy(r[1]);
  m_style     = get_style(r[3]);
  m_line_no   = std::stoi(r[4]) - 1;
  m_line_text = r[5];

  return true;
}
