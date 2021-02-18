////////////////////////////////////////////////////////////////////////////////
// Name:      core/regex.cpp
// Purpose:   Implementation of class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <wex/log.h>
#include <wex/regex.h>

wex::regex::regex(const std::string& regex)
  : m_regex({regex})
{
}

wex::regex::regex(const std::vector<std::string>& regex)
  : m_regex(regex)
{
}

int wex::regex::match(const std::string& text)
{
  if (m_regex.empty())
  {
    return -1;
  }

  for (const auto& reg : m_regex)
  {
    try
    {
      if (std::match_results<std::string::const_iterator> m;
          std::regex_search(text, m, std::regex(reg)))
      {
        if (m.size() > 1)
        {
          m_matches.clear();
          std::copy(++m.begin(), m.end(), std::back_inserter(m_matches));
        }

        return m_matches.size();
      }
    }
    catch (std::regex_error& e)
    {
      log(e) << reg << "code:" << (int)e.code();
    }
  }

  return -1;
}
