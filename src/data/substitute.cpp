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

wex::data::substitute::substitute(
  const std::string& pattern,
  const std::string& replacement,
  const std::string& options)
  : m_pattern(pattern)
  , m_replacement(replacement)
  , m_options(options)
{
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

bool wex::data::substitute::set(
  const std::string& command_org,
  const std::string& pattern)
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
    m_pattern = r[0];

    if (r.size() >= 2)
      m_replacement = r[1];
    if (r.size() >= 3)
      m_options = r[2];

    // Restore a / for all occurrences of the special char.
    if (escaped)
    {
      std::replace(m_pattern.begin(), m_pattern.end(), '\x01', '/');
      std::replace(m_replacement.begin(), m_replacement.end(), '\x01', '/');
    }

    if (m_pattern.empty())
    {
      m_pattern = pattern;
    }

    return true;
  }

  return false;
}
