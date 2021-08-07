////////////////////////////////////////////////////////////////////////////////
// Name:      blame.cpp
// Purpose:   Implementation of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/blame.h>
#include <wex/chrono.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/regex.h>

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

std::tuple<bool, const std::string, wex::lexers::margin_style_t, int>
wex::blame::get(const std::string& text) const
{
  try
  {
    if (regex r(m_blame_format); r.search(text) >= 3)
    {
      const std::string info(
        build("id", r[0], true) + build("author", r[1]) +
        build("date", r[2].substr(0, m_date_print)));

      const auto line(r.size() == 4 ? std::stoi(r[3]) - 1 : -1);

      return {true, info.empty() ? " " : info, get_style(r[2]), line};
    }
  }
  catch (std::exception& e)
  {
    log(e) << "blame:" << text;
  }

  return {false, std::string(), lexers::margin_style_t::OTHER, 0};
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
