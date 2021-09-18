////////////////////////////////////////////////////////////////////////////////
// Name:      config-imp.h
// Purpose:   Implementation of class wex::config_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/json.hpp>

#include <wex/core.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/tokenize.h>

namespace json = boost::json;

namespace wex
{
/// This class implements the wex config, by using boost json.
class config_imp
{
public:
  /// Static interface.

  /// Returns config path.
  static auto path() { return m_path; }

  /// Sets config path to use. If not called,
  /// the default is used.
  static void set_path(const wex::path& p) { m_path = p; }

  /// Other methods.

  /// Default constructor.
  config_imp();

  /// Copy constructor with a possible child item.
  config_imp(const config_imp* c, const std::string& item = std::string());

  /// Returns number of elements.
  size_t elements() const;

  /// Returns true if this item is present.
  bool exists(const std::string& item) const;

  /// Returns item.
  auto& get_item() const { return m_item; }

  /// Returns boost json object.
  const auto& get_json() const { return m_json; }
  auto&       get_json() { return m_json; }

  /// Reads json file.
  void read();

  /// Saves json file.
  void save() const;

  /// Sets value for item.
  /// set("x.y.z", 8)
  /// -> boost json object x, y boost json value z 8
  /// set("x.y.z", std::vector{1, 2 3})
  /// -> boost json object x, y, z
  template <typename T> void set(const std::string& item, const T& v);

  /// Returns value for item.
  /// If item is present, returns current value, otherwise the default one.
  /// value("x.y.z", 9) -> 9
  /// set("x.y.z", 8)
  /// value("x.y.z", 9) -> 8
  template <typename T> const T value(const std::string& item, const T& def);

private:
  template <typename T>
  json::value& accessor(const std::string& item, const T& def);

  void elements(const json::object& o, size_t& total) const;

  json::object            m_json;
  static inline wex::path m_path;
  const std::string       m_item;
};
}; // namespace wex

// implementation

template <typename T>
json::value& wex::config_imp::accessor(const std::string& item, const T& t)
{
  if (const auto& v(tokenize<std::vector<std::string>>(item, "."));
      v.size() == 0)
  {
    return m_json[item];
  }
  else
  {
    auto* jv(&m_json[v[0]]);

    for (size_t i = 1; i < v.size(); i++)
    {
      if (!jv->is_null() && jv->is_object())
      {
        jv = &jv->as_object()[v[i]];
      }
      else
      {
        jv = &jv->emplace_object()[v[i]];
      }
    }

    if (jv->is_null())
    {
      *jv = json::value_from(t);
    }

    return *jv;
  }
}

template <typename T>
void wex::config_imp::set(const std::string& item, const T& v)
{
  try
  {
    if (item.find('.') == std::string::npos)
    {
      m_json[item] = json::value_from(v);
    }
    else
    {
      if (exists(item))
      {
        accessor(item, v) = json::value_from(v);
      }
      else
      {
        auto& jv = accessor(before(item, '.', false), v);

        if (jv.is_object())
        {
          jv.as_object()[after(item, '.', false)] = json::value_from(v);
        }
        else
        {
          jv.emplace_object()[after(item, '.', false)] = json::value_from(v);
        }
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << "config::set" << item;
  }
}

template <typename T>
const T wex::config_imp::value(const std::string& item, const T& def)
{
  const auto& ai(after(item, '.', false));

  try
  {
    if (item.find('.') == std::string::npos)
    {
      if (!exists(item))
      {
        return def;
      }
      else
      {
        const auto& jv(m_json.at(item));
        return json::value_to<T>(jv);
      }
    }
    else
    {
      if (auto& a(accessor(before(item, '.', false), def)); !a.is_null())
      {
        if (a.is_object())
        {
          if (!a.as_object().contains(ai))
          {
            a.as_object()[ai] = json::value_from(def);
            return def;
          }
          else
          {
            return json::value_to<T>(a.as_object()[ai]);
          }

          if (const auto& l(a.at(ai)); !l.is_null())
          {
            return json::value_to<T>(l);
          }
        }

        a.emplace_object()[ai] = json::value_from(def);
        return def;
      }
      else
      {
        a.emplace_object()[ai] = json::value_from(def);
        return def;
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << "config::value" << item;
    return def;
  }
}
