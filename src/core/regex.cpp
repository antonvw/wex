////////////////////////////////////////////////////////////////////////////////
// Name:      core/regex.cpp
// Purpose:   Implementation of class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>

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

wex::regex::regex(const data& d)
  : m_datas({d})
  , m_it(m_datas.end())
{
}

wex::regex::regex(const std::string& str, std::regex::flag_type flags)
  : m_datas({{{str, nullptr}, flags}})
  , m_it(m_datas.end())
{
}

wex::regex::regex(
  const std::string&    str,
  function_t            f,
  std::regex::flag_type flags)
  : m_datas({{{str, f}, flags}})
  , m_it(m_datas.end())
{
}

wex::regex::regex(const regex_v_t& regex, std::regex::flag_type flags)
  : m_datas(FILL_DATA(regex_v_t, v.emplace_back(regex_c_t(e, nullptr), flags)))
  , m_it(m_datas.end())
{
}

wex::regex::regex(const regex_v_c_t& regex, std::regex::flag_type flags)
  : m_datas(FILL_DATA(regex_v_c_t, v.emplace_back(e, flags)))
  , m_it(m_datas.end())
{
}

const std::string wex::regex::operator[](size_t pos) const 
{ 
  return pos >= m_matches.size() ? std::string(): m_matches[pos]; 
}

const std::string wex::regex::back() const 
{ 
 return m_matches.empty() ? std::string(): m_matches.back(); 
}
int wex::regex::find(const std::string& text, find_t how)
{
  m_it = std::find_if(
    m_datas.begin(),
    m_datas.end(),
    [this, text, how](auto const& reg)
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

        if (reg.function() != nullptr)
        {
          reg.function()(m_matches);
        }

        return true;
      }
      else
      {
        return false;
      }
    });

  return (m_it != m_datas.end()) ? static_cast<int>(m_matches.size()) : -1;
}

int wex::regex::match(const std::string& text)
{
  return find(text, find_t::MATCH);
}

const wex::regex::data wex::regex::match_data() const
{
  return m_it != m_datas.end() ? *m_it : data();
}

int wex::regex::match_no() const
{
  return m_it == m_datas.end() ? -1 : std::distance(m_datas.begin(), m_it);
}

bool wex::regex::replace(
  std::string&                          text,
  const std::string&                    replacement,
  std::regex_constants::match_flag_type flag_type) const
{
  if (m_it == m_datas.end())
  {
    return false;
  }

  text = std::regex_replace(text, m_it->regex(), replacement, flag_type);

  return true;
}

int wex::regex::search(const std::string& text)
{
  return find(text, find_t::SEARCH);
}

wex::regex::data::data(const regex_c_t& regex, std::regex::flag_type flags)
  : m_text(regex.first)
  , m_function(regex.second)
{
  init(regex, flags);
}

wex::regex::data::data() {}

void wex::regex::data::init(const regex_c_t& regex, std::regex::flag_type flags)
{
  try
  {
    m_regex = std::regex(m_text, flags);
  }
  catch (std::regex_error& e)
  {
    log(e) << m_text << "code:" << static_cast<int>(e.code());
  }
}
