////////////////////////////////////////////////////////////////////////////////
// Name:      core/regex.cpp
// Purpose:   Implementation of class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>
#include <wex/core/regex.h>

#define FILL_DATA(TYPE, ACTION)                      \
  [](const TYPE& reg_v, std::regex::flag_type flags) \
  {                                                  \
    regex_t v;                                       \
    std::for_each(                                   \
      reg_v.begin(),                                 \
      reg_v.end(),                                   \
      [&v, flags](const auto& e)                     \
      {                                              \
        ACTION;                                      \
      });                                            \
    return v;                                        \
  }(regex, flags)

enum class wex::regex::find_t
{
  SEARCH,
  MATCH,
};

wex::regex::regex(const std::string& str, std::regex::flag_type flags)
  : m_datas({{{str, nullptr}, flags}})
{
}

wex::regex::regex(
  const std::string&    str,
  function_t            f,
  std::regex::flag_type flags)
  : m_datas({{{str, f}, flags}})
{
}

wex::regex::regex(const regex_v_t& regex, std::regex::flag_type flags)
  : m_datas(FILL_DATA(regex_v_t, v.emplace_back(regex_c_t(e, nullptr), flags)))
{
}

wex::regex::regex(const regex_v_c_t& regex, std::regex::flag_type flags)
  : m_datas(FILL_DATA(regex_v_c_t, v.emplace_back(e, flags)))
{
}

int wex::regex::find(const std::string& text, find_t how)
{
  m_match_no = -1;

  if (m_datas.empty())
  {
    return -1;
  }

  for (int index = 0; const auto& reg : m_datas)
  {
    try
    {
      if (std::match_results<std::string::const_iterator> m;
          ((how == find_t::MATCH && std::regex_match(text, m, reg.regex())) ||
           (how == find_t::SEARCH && std::regex_search(text, m, reg.regex()))))
      {
        if (m.size() > 1)
        {
          m_matches.clear();
          std::copy(++m.begin(), m.end(), std::back_inserter(m_matches));
        }

        m_data     = reg;
        m_match_no = index;

        if (reg.function() != nullptr)
        {
          reg.function()(m_matches);
        }

        return static_cast<int>(m_matches.size());
      }

      index++;
    }
    catch (std::regex_error& e)
    {
      log(e) << reg.text() << "code:" << static_cast<int>(e.code());
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
  if (m_data.text().empty())
  {
    return false;
  }

  text = std::regex_replace(text, m_data.regex(), replacement, flag_type);

  return true;
}

int wex::regex::search(const std::string& text)
{
  return find(text, find_t::SEARCH);
}

wex::regex::data::data(const regex_c_t& regex, std::regex::flag_type flags)
  : m_regex(regex.first, flags)
  , m_function(regex.second)
  , m_text(regex.first)
{
}
