////////////////////////////////////////////////////////////////////////////////
// Name:      data/substitute.cpp
// Purpose:   Implementation of class wex::data::substitute
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/data/substitute.h>
#include <wex/log.h>
#include <wex/regex.h>

wex::data::substitute::substitute(const std::string& text)
{
  set(text);
}

bool wex::data::substitute::is_confirmed() const
{
  return m_options.find("c") != std::string::npos;
}

bool wex::data::substitute::is_global() const
{
  return m_options.find("g") != std::string::npos;
}

bool wex::data::substitute::is_ignore_case() const
{
  return m_options.find("i") != std::string::npos;
}

bool wex::data::substitute::set(const std::string& command_org)
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use regex with / as search character.
  bool escaped = false;

  auto command(command_org);

  if (
    command.find("\\\\/") == std::string::npos &&
    command.find("\\/") != std::string::npos)
  {
    if (command.find(char(1)) == std::string::npos)
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
  regex v("^(\\s*)/(.*?)/(.*)");

  // [2addr] g[lobal] /pattern/ [commands]
  // [2addr] v /pattern/ [commands]
  // the g or v part is already parsed, and not present, v[0] is empty, or ws
  if (v.match(text) < 3)
  {
    return false;
  }

  m_pattern  = v[1];
  m_commands = v[2];

  if (m_pattern.empty())
  {
    if (!m_commands.empty())
    {
      log::status("Pattern is empty");
      return false;
    }
  }

  return true;
}

void wex::data::substitute::set_options(const std::string& text)
{
  m_options = text;
}
