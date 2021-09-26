////////////////////////////////////////////////////////////////////////////////
// Name:      core/regex.cpp
// Purpose:   Implementation of class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/core/regex.h>

enum class wex::regex::find_t
{
  SEARCH,
  MATCH,
};

wex::regex::regex(const std::string& str, std::regex::flag_type flags)
  : m_regex({{std::regex(str.c_str(), flags), nullptr, str}})
{
}

wex::regex::regex(
  const std::string&    str,
  function_t            f,
  std::regex::flag_type flags)
  : m_regex({{std::regex(str.c_str(), flags), f, str}})
{
}

wex::regex::regex(const regex_v_t& regex, std::regex::flag_type flags)
  : m_regex(
      [](const regex_v_t& reg_str, std::regex::flag_type flags)
      {
        regex_t v;

        for (const auto& r : reg_str)
        {
          v.emplace_back(std::regex(r, flags), nullptr, r);
        }

        return v;
      }(regex, flags))
{
}

wex::regex::regex(const regex_v_c_t& regex, std::regex::flag_type flags)
  : m_regex(
      [](const regex_v_c_t& reg_str, std::regex::flag_type flags)
      {
        regex_t v;

        for (const auto& r : reg_str)
        {
          v.emplace_back(std::regex(r.first, flags), r.second, r.first);
        }

        return v;
      }(regex, flags))
{
}

int wex::regex::find(const std::string& text, find_t how)
{
  m_which_no = -1;

  if (m_regex.empty())
  {
    return -1;
  }

  int index = 0;

  for (const auto& reg : m_regex)
  {
    try
    {
      if (std::match_results<std::string::const_iterator> m;
          ((how == find_t::MATCH &&
            std::regex_match(text, m, std::get<0>(reg))) ||
           (how == find_t::SEARCH &&
            std::regex_search(text, m, std::get<0>(reg)))))
      {
        if (m.size() > 1)
        {
          m_matches.clear();
          std::copy(++m.begin(), m.end(), std::back_inserter(m_matches));
        }

        m_which    = reg;
        m_which_no = index;

        if (std::get<1>(reg) != nullptr)
        {
          std::get<1>(reg)(m_matches);
        }

        return static_cast<int>(m_matches.size());
      }

      index++;
    }
    catch (std::regex_error& e)
    {
      log(e) << std::get<2>(reg) << "code:" << static_cast<int>(e.code());
    }
  }

  return -1;
}

int wex::regex::match(const std::string& text)
{
  return find(text, find_t::MATCH);
}

bool wex::regex::replace(
  std::string&                          text,
  const std::string&                    replacement,
  std::regex_constants::match_flag_type flag_type) const
{
  if (std::get<2>(m_which).empty())
  {
    return false;
  }

  text = std::regex_replace(text, std::get<0>(m_which), replacement, flag_type);

  return true;
}

int wex::regex::search(const std::string& text)
{
  return find(text, find_t::SEARCH);
}
