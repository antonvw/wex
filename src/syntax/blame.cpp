////////////////////////////////////////////////////////////////////////////////
// Name:      blame.cpp
// Purpose:   Implementation of class wex::blame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <charconv>

#include <wex/core/chrono.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/core/regex.h>
#include <wex/factory/stc.h>
#include <wex/syntax/blame.h>

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

    if (key == "author")
    {
      auto text(boost::algorithm::trim_copy(field));

      if (const int shown(config(_("blame.Author size")).get(-1)); shown != -1)
      {
        text = text.substr(0, shown);
      }

      add += text;
    }
    else
    {
      add += boost::algorithm::trim_copy(field);
    }
  }

  return add;
}

const std::string renamed(":::");
} // namespace wex

wex::blame::blame(const pugi::xml_node& node)
  : m_blame_format(node.attribute("blame-format").value())
  , m_date_format(node.attribute("date-format").value())
  , m_date_print(node.attribute("date-print").as_uint())
  , m_name(node.attribute("name").value())
  , m_path_original("xxxxx")
{
}

wex::blame::margin_style_t wex::blame::get_style(const std::string& text) const
{
  margin_style_t style = margin_style_t::UNKNOWN;

  if (text.empty())
  {
    return style;
  }

  if (const auto& r(chrono(m_date_format).get_time(text)); r)
  {
    const time_t now               = time(nullptr);
    const auto   dt                = difftime(now, *r);
    const int    seconds           = 1;
    const int    seconds_in_minute = 60 * seconds;
    const int    seconds_in_hour   = 60 * seconds_in_minute;
    const int    seconds_in_day    = 24 * seconds_in_hour;
    const int    seconds_in_week   = 7 * seconds_in_day;
    const int    seconds_in_month  = 30 * seconds_in_day;
    const int    seconds_in_year   = 365 * seconds_in_day;

    if (dt < seconds_in_day)
    {
      style = margin_style_t::DAY;
    }
    else if (dt < seconds_in_week)
    {
      style = margin_style_t::WEEK;
    }
    else if (dt < seconds_in_month)
    {
      style = margin_style_t::MONTH;
    }
    else if (dt < seconds_in_year)
    {
      style = margin_style_t::YEAR;
    }
    else
    {
      style = margin_style_t::OTHER;
    }
  }

  return style;
}

const std::string wex::blame::info() const
{
  return m_info + (is_renamed_path() ? renamed + m_path : std::string());
}

bool wex::blame::is_renamed_path() const
{
  return m_path != m_path_original.string() && !m_path.empty();
}

std::string wex::blame::margin_renamed(const factory::stc* stc)
{
  std::string revision(stc->MarginGetText(stc->get_margin_text_click()));

  return !revision.contains(renamed) ? std::string() :
                                       find_after(revision, renamed);
}

bool wex::blame::parse(const path& p, const std::string& text)
{
  m_path_original = p;

  try
  {
    if (regex r(m_blame_format); r.search(text) >= 4)
    {
      if (m_name == "svn" && r.size() == 4)
      {
        return parse_compact(text, r);
      }

      if (r.size() == 6)
      {
        return parse_full(text, r);
      }

      log("blame parsing") << m_name << r.size();
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
// 3 -> original line text
bool wex::blame::parse_compact(const std::string& line, const regex& r)
{
  m_info = build("id", r[0], true) + build("author", r[1]) +
           build("date", r[2].substr(0, m_date_print));

  if (m_info.empty())
  {
    m_info = " ";
  }

  m_skip_info = false;
  m_path.clear(); // not present in svn blame
  m_style = get_style(r[2]);
  m_line_no++; // not present in svn blame
  m_line_text = r[3];

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

  if (const auto [ptr, ec] =
        std::from_chars(r[4].data(), r[4].data() + r[4].size(), m_line_no);
      ec == std::errc())
  {
    m_line_no--;
    m_line_text = r[5];

    return true;
  }

  return false;
}
