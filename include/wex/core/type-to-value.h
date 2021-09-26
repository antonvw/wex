////////////////////////////////////////////////////////////////////////////////
// Name:      type-to-value.h
// Purpose:   Declaration of class wex::type_to_value
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/log.h>

import<string>;

namespace wex
{
/// Convert a general type to string, for int or string type.
template <typename T> class type_to_value
{
public:
  /// Constructor, using string argument.
  explicit type_to_value(const std::string& v)
    : m_v(v)
  {
    ;
  };

  /// Returns value as a string.
  const auto& get() const { return m_v; }

  /// Returns value as a string.
  const auto& get_string() const { return m_v; }

private:
  const std::string& m_v;
};

/// Convert a general type to int.
template <> class type_to_value<int>
{
public:
  /// Constructor, using int argument.
  explicit type_to_value(int v)
    : m_i(v)
  {
    ;
  };

  /// Constructor, using string argument.
  explicit type_to_value(const std::string& v)
    : m_s(v)
  {
    ;
  };

  /// Returns value as an integer.
  int get() const
  {
    if (m_i != 0)
      return m_i;

    try
    {
      const char c = m_s.front();

      if (!isdigit(c))
      {
        return c;
      }
      else
      {
        return std::stoi(m_s);
      }
    }
    catch (std::exception& e)
    {
      log(e) << "value:" << m_s;
      return m_i;
    }
  };

  /// Returns value a string.
  const auto get_string() const
  {
    if (!m_s.empty())
      return m_s;

    try
    {
      if (isalnum(m_i) || isgraph(m_i))
      {
        return std::string(1, m_i);
      }
      else if (iscntrl(m_i))
      {
        return "ctrl-" + std::string(1, m_i + 64);
      }
      else
      {
        return std::to_string(m_i);
      }
    }
    catch (std::exception& e)
    {
      log(e) << "value:" << m_i;
      return m_s;
    }
  };

private:
  const int         m_i{0};
  const std::string m_s{std::string()};
};
}; // namespace wex
