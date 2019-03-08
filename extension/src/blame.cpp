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
  void build(std::string& text, const std::string& append)
  {
    if (!text.empty()) 
    {
      text += " ";
    }
    
    text += append;
  };

  const std::string from(
    const blame::field& x, 
    const blame::field& y, const std::string& text)
  { 
    const auto begin = x.pos(text);
    const auto end = y.pos(text);

    if (begin == std::string::npos && end == std::string::npos)
    {
      return std::string();  
    }  
    
    const int xx = !x.is_number() ? 1: 0;
    
    if (begin + xx >= text.size())
    {
      return std::string();
    }

    return text.substr(begin + xx, end - begin - 1);
  };
}
  
wex::blame::field::field(const std::string& value)
  : m_value(value)
{
  try
  {
    m_number = std::stoi(value);
  }
  catch (std::exception& )
  {
    m_number = std::string::npos;
  }
}

size_t wex::blame::field::pos(const std::string text) const
{
  return is_number() ? m_number: text.find(m_value);
}

wex::blame::blame(const pugi::xml_node& node)
  : m_date_format(node.attribute("date-format").value())
  , m_date_print(node.attribute("date-print").as_uint())
  , m_pos_author_begin(node.attribute("pos-author-begin").value())
  , m_pos_begin(node.attribute("pos-begin").value())
  , m_pos_end(node.attribute("pos-end").value())
  , m_pos_id_begin(node.attribute("pos-id-begin").value())
  , m_pos_id_end(node.attribute("pos-id-end").value())
{
}
  
std::tuple <bool, const std::string, wex::lexers::margin_style_t> 
  wex::blame::get(const std::string& text) const
{
  if (const auto blame_text(from(m_pos_begin, m_pos_end, text)); blame_text.empty())
  {
    return {false, std::string(), lexers::MARGIN_STYLE_OTHER};
  }
  else
  {
    std::string info;
  
    if (config("blame_get_id").get(true))
    {
      build(info, get_id(blame_text));
    }
  
    const auto date(get_date(blame_text));
  
    if (!date.empty() && config("blame_get_date").get(true))
    {
      build(info, date.substr(0, m_date_print));
    }

    if (config("blame_get_author").get(true))
    {
      build(info, get_author(blame_text));
    }
    
    if (info.empty())
    {
      info = " ";
    }
  
    return {true, info, get_style(date)};
  }
}

const std::string wex::blame::get_author(const std::string& text) const
{
  const std::string search(from(m_pos_author_begin, m_pos_end, text));

  if (std::vector<std::string> v;
    match("([a-zA-Z ]+)", search, v) > 0)
  {
    return skip_white_space(v[0]);
  }

  log("author") << search;
  
  return std::string();
}

const std::string wex::blame::get_date(const std::string& text) const
{
  if (std::vector<std::string> v;
    match("([0-9]{2,4}.[0-9]{2}.[0-9]{2}.[0-9:]{8})", text, v) > 0)
  {
    return v[0];
  }

  log("date") << text;
  
  return std::string();
}

const std::string wex::blame::get_id(const std::string& text) const
{
  return from(m_pos_id_begin, m_pos_id_end, text);
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

bool wex::blame::use() const 
{
  return !m_pos_begin.value().empty();
}
