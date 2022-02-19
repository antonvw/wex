////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-input.cpp
// Purpose:   Implementation of wex::textctrl_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ui/frd.h>
#include <wex/ui/textctrl-input.h>
#include <wex/ui/textctrl.h>

#include <charconv>

wex::textctrl_input::textctrl_input(
  ex_command::type_t type,
  const std::string& name)
  : m_type(type)
  , m_name(name)
  , m_values(config(m_name).get(values_t{}))
  , m_iterator(m_values.cbegin())
{
  if (!m_values.empty())
  {
    log::trace("load") << m_name << "size:" << m_values.size();
  }
}

wex::textctrl_input::~textctrl_input()
{
  if (!m_values.empty())
  {
    const int max_ints{5};
    values_t  filtered;

    for (int current{0}; const auto& v : m_values)
    {
      // If this value is an int, ignore value if we reached max
      if (int value = 0;
          std::from_chars(v.data(), v.data() + v.size(), value).ec ==
          std::errc())
      {
        if (current++ >= max_ints)
        {
          continue;
        }
      }

      filtered.emplace_back(v);
    }

    std::stringstream info;

    if (const auto diff(m_values.size() - filtered.size()); diff > 0)
    {
      info << "filtered:" << diff;
    }

    wex::config(m_name).set(filtered);
    log::trace("save") << m_name << "size:" << filtered.size() << info;
  }
}

const std::string wex::textctrl_input::get() const
{
  try
  {
    return m_iterator != m_values.end() ? *m_iterator : std::string();
  }
  catch (std::exception& e)
  {
    log(e) << m_name;
    return std::string();
  }
}

void wex::textctrl_input::set(const std::string& value)
{
  assert(!value.empty());

  m_values.remove(value);
  m_values.push_front(value);
  m_iterator = m_values.cbegin();

  config(m_name).set(m_values);
}

void wex::textctrl_input::set(const textctrl* tc)
{
  if (const auto v(tc->get_text()); !v.empty())
  {
    set(v);
  }
}

bool wex::textctrl_input::set(int key, textctrl* tc)
{
  if (m_values.empty())
  {
    return false;
  }

  switch (const int page = 10; key)
  {
    case WXK_DOWN:
      if (m_iterator != m_values.cbegin())
      {
        --m_iterator;
      }
      break;

    case WXK_UP:
      if (m_iterator != m_values.cend())
      {
        ++m_iterator;
      }
      break;

    case WXK_END:
      m_iterator = m_values.cend();
      --m_iterator;
      break;

    case WXK_HOME:
      m_iterator = m_values.cbegin();
      break;

    case WXK_PAGEDOWN:
      if (std::distance(m_values.cbegin(), m_iterator) > page)
      {
        std::advance(m_iterator, -page);
      }
      else
      {
        m_iterator = m_values.cbegin();
      }
      break;

    case WXK_PAGEUP:
      if (std::distance(m_iterator, m_values.cend()) > page)
      {
        std::advance(m_iterator, page);
      }
      else
      {
        m_iterator = m_values.cend();
        --m_iterator;
      }
      break;

    default:
      return false;
  }

  if (tc != nullptr)
  {
    tc->set_text(get());
    tc->select_all();
  }

  return true;
}

void wex::textctrl_input::set(const values_t& values)
{
  m_values.assign(values.cbegin(), values.cend());
  m_iterator = m_values.cbegin();
  config(m_name).set(values);
}
