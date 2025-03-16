////////////////////////////////////////////////////////////////////////////////
// Name:      data/substitute.cpp
// Purpose:   Implementation of class wex::data::substitute
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/data/substitute.h>

wex::data::substitute::substitute(const std::string& text)
{
  set(text);
}

bool wex::data::substitute::is_confirmed() const
{
  return m_options.contains("c");
}

bool wex::data::substitute::is_global() const
{
  return m_options.contains("g");
}

bool wex::data::substitute::is_ignore_case() const
{
  return m_options.contains("i");
}

bool wex::data::substitute::set(const std::string& command_org)
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use regex with / as search character.
  bool escaped = false;

  auto command(command_org);

  if (!command.contains("\\\\/") && command.contains("\\/"))
  {
    if (!command.contains(static_cast<char>(1)))
    {
      boost::algorithm::replace_all(command, "\\/", "\x01");
      escaped = true;
    }
    else
    {
      log::debug("internal char exists") << command;
      return false;
    }
  }

  m_global_command = false;

  if (regex r({{"/(.*)/(.*)/([cgi]*)"}, {"/(.*)/(.*)"}, {"/(.*)"}});
      r.search(command) > 0)
  {
    if (!r[0].empty() && r[0] != "~")
    {
      m_pattern = r[0];
    }

    if (r.size() >= 2 && r[0] != "~" && r[1] != "~")
    {
      m_replacement = r[1];
    }

    if (r.size() >= 3)
    {
      m_options = r[2];
    }

    // Restore a / for all occurrences of the special char.
    if (escaped)
    {
      std::replace(m_pattern.begin(), m_pattern.end(), '\x01', '/');
      std::replace(m_replacement.begin(), m_replacement.end(), '\x01', '/');
    }

    return true;
  }

  return false;
}

bool wex::data::substitute::set_global(const std::string& text)
{
  // [2addr] g[lobal] /pattern/ [commands]
  // [2addr] v /pattern/ [commands]
  // The optional '!' character after the global command shall be the same as
  // executing the v command.
  regex v("^(\\s*[gv]|global|global!|g!)/(.*?)/(.*)"); // non-greedy

  if (v.match(text) < 3)
  {
    return false;
  }

  m_global_command = true;
  m_inverse        = v[0].starts_with('v') || v[0].ends_with('!');
  auto pattern     = v[1];
  m_commands       = v[2];

  if (pattern.empty())
  {
    // an empty pattern refers to the previous pattern.
    pattern = m_pattern;

    m_clear = (m_commands.empty());
  }
  else
  {
    m_clear = false;
  }

  m_pattern = pattern;

  if (m_pattern.empty())
  {
    log::status("Pattern is empty");
    return false;
  }

  return true;
}

void wex::data::substitute::set_options(const std::string& text)
{
  m_options = text;
}
