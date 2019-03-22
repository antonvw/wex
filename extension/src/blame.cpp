////////////////////////////////////////////////////////////////////////////////
// Name:      blame.cpp
// Purpose:   Implementation of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/blame.h>
#include <wex/config.h>
#include <wex/util.h>

namespace wex
{
  void build(const std::string& key, std::string& text, const std::string& append)
  {
    if (config(key).get(true))
    {
      if (!text.empty()) 
      {
        text += " ";
      }
      
      text += append;
    }
  }
}
  
wex::blame::blame(const pugi::xml_node& node)
  : m_blame_format(node.attribute("blame-format").value())
  , m_date_format(node.attribute("date-format").value())
  , m_date_print(node.attribute("date-print").as_uint())
{
}
  
std::tuple <bool, const std::string, wex::lexers::margin_style_t> 
  wex::blame::get(const std::string& text) const
{
  if (std::vector<std::string> v; match(m_blame_format, text, v) == 3)
  {
    std::string info;
  
    build("blame_get_id", info, v[0]);
    build("blame_get_author", info, v[1]);
    build("blame_get_date", info, v[2].substr(0, m_date_print));

    return {true, info.empty() ? " ": info, get_style(v[2])};
  }
  
  return {false, std::string(), lexers::MARGIN_STYLE_OTHER};
}

wex::lexers::margin_style_t wex::blame::get_style(const std::string& text) const
{
  lexers::margin_style_t style = lexers::MARGIN_STYLE_UNKNOWN;

  if (text.empty())
  {
    return style;
  }
  
  if (const auto& [r, t] = get_time(text, m_date_format); r)
  {
    const time_t now = time(nullptr);
    const auto dt = difftime(now, t);
    const int seconds = 1;
    const int seconds_in_minute = 60 * seconds;
    const int seconds_in_hour = 60 * seconds_in_minute;
    const int seconds_in_day = 24 * seconds_in_hour;
    const int seconds_in_week = 7 * seconds_in_day;
    const int seconds_in_month = 30 * seconds_in_day;
    const int seconds_in_year = 365 * seconds_in_day;

    if (dt < seconds_in_day)
    {
      style = lexers::MARGIN_STYLE_DAY;
    }
    else if (dt < seconds_in_week)
    {
      style = lexers::MARGIN_STYLE_WEEK;
    }
    else if (dt < seconds_in_month)
    {
      style = lexers::MARGIN_STYLE_MONTH;
    }
    else if (dt < seconds_in_year)
    {
      style = lexers::MARGIN_STYLE_YEAR;
    }
    else
    {
      style = lexers::MARGIN_STYLE_OTHER;
    }
  }
  else
  {
    log("date") << text << "format:" << m_date_format;
  }
        
  return style;
}
