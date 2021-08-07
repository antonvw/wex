////////////////////////////////////////////////////////////////////////////////
// Name:      config-imp.h
// Purpose:   Implementation of class wex::config_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <nlohmann/json.hpp>
#include <string>

#include <wex/core.h>
#include <wex/log.h>
#include <wex/path.h>

using json = nlohmann::json;

namespace wex
{
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
  bool exists(const std::string& item);

  /// Returns item.
  auto& get_item() const { return m_item; }

  /// Returns json.
  const json& get_json() const { return m_json; }
  json&       get_json() { return m_json; }

  /// Reads json file.
  void read();

  /// Saves json file.
  void save() const;

  /// Sets value for item.
  template <typename T> void set(const std::string& item, const T& v)
  {
    try
    {
      if (item.find('.') == std::string::npos)
      {
        m_json[item] = v;
      }
      else
      {
        accessor(before(item, '.', false))[after(item, '.', false)] = v;
      }
    }
    catch (std::exception& e)
    {
      log(e) << "config::set" << item;
    }
  }

  /// Returns value for item.
  template <typename T> const T value(const std::string& item, const T& def)
  {
    try
    {
      if (item.find('.') == std::string::npos)
      {
        return m_json.value(item, def);
      }
      else
      {
        auto& a(accessor(before(item, '.', false)));
        return !a.is_null() ? a.value(after(item, '.', false), def) : def;
      }
    }
    catch (std::exception& e)
    {
      log(e) << "config::value" << item;
      return def;
    }
  }

private:
  json& accessor(const std::string& item);
  void  elements(const json& o, size_t& total) const;

  json                    m_json;
  static inline wex::path m_path;
  const std::string       m_item;
};
}; // namespace wex
