////////////////////////////////////////////////////////////////////////////////
// Name:      core/regex.cpp
// Purpose:   Implementation of class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/log.h>
#include <wex/regex.h>

wex::regex::regex(const std::string& str, std::regex::flag_type flags)
  : m_regex({{std::regex(str.c_str(), flags), str}})
{
}

wex::regex::regex(
  const std::vector<std::string>& regex,
  std::regex::flag_type           flags)
  : m_regex(
      [](const std::vector<std::string>& reg_str, std::regex::flag_type flags) {
        regex_t v;

        for (const auto& r : reg_str)
        {
          v.emplace_back(std::regex(r, flags), r);
        }

        return v;
      }(regex, flags))
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
          std::regex_search(text, m, reg.first))
      {
        if (m.size() > 1)
        {
          m_matches.clear();
          std::copy(++m.begin(), m.end(), std::back_inserter(m_matches));
        }

        m_which = reg;

        return m_matches.size();
      }
    }
    catch (std::regex_error& e)
    {
      log(e) << reg.second << "code:" << (int)e.code();
    }
  }

  return -1;
}
