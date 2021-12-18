////////////////////////////////////////////////////////////////////////////////
// Name:      config-imp.cpp
// Purpose:   Implementation of class wex::config_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/json/src.hpp>

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wx/app.h>

#include "config-imp.h"

#include <fstream>
#include <iomanip>

wex::config_imp::config_imp()
  : m_json({})
{
}

wex::config_imp::config_imp(const config_imp* c, const std::string& item)
  : m_json(c == nullptr ? json::object() : c->m_json)
  , m_item(item)
{
  if (!item.empty())
  {
    if (const auto& it = m_json.find(item);
        it != m_json.end() && it->value().is_object())
    {
      m_json = it->value().as_object();
    }
    else
    {
      m_json = json::object();
    }
  }
}

size_t wex::config_imp::elements() const
{
  size_t total = 0;
  elements(m_json, total);
  return total;
}

void wex::config_imp::elements(const json::object& o, size_t& total) const
{
  total += o.size();

  for (const auto& x : o)
  {
    if (x.value().is_object())
    {
      elements(x.value().as_object(), total);
    }
  }
}

bool wex::config_imp::exists(const std::string& item) const
{
  if (item.find('.') == std::string::npos)
  {
    return m_json.contains(item);
  }
  else if (const auto& v(tokenize<std::vector<std::string>>(item, "."));
           v.size() == 0)
  {
    return m_json.contains(item);
  }
  else
  {
    const auto& it = m_json.find(v[0]);

    if (it == m_json.end())
    {
      return false;
    }
    else
    {
      const auto* jv(&it->value());

      for (size_t i = 1; i < v.size(); i++)
      {
        if (!jv->is_null() && jv->is_object())
        {
          const auto& it = jv->as_object().find(v[i]);

          if (it == jv->as_object().end())
          {
            return false;
          }

          jv = &it->value();
        }
      }

      return !jv->is_null();
    }
  }
}

void wex::config_imp::read()
{
  if (m_path.empty())
  {
    m_path = wex::path(config::dir(), wxTheApp->GetAppName().Lower() + ".json");
  }

  if (wex::file fs(m_path, std::ios_base::in); fs.is_open())
  {
    try
    {
      if (const auto buffer(fs.read()); buffer != nullptr)
      {
        m_json = json::parse(*buffer).as_object();

        log::trace("config") << m_path.string() << "top size:" << m_json.size()
                             << "elements:" << elements();
      }
    }
    catch (std::exception& e)
    {
      log(e) << m_path;
    }
  }
  else
  {
    log::debug("could not read") << m_path;
  }
}

void wex::config_imp::save() const
{
  if (std::ofstream fs(m_path.string()); fs.is_open())
  {
    fs << std::setw(2) << m_json << std::endl;
  }
  else
  {
    log("could not save") << m_path;
  }
}
