////////////////////////////////////////////////////////////////////////////////
// Name:      config-imp.cpp
// Purpose:   Implementation of class wex::config_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iomanip>
#include <nlohmann/json.hpp>

#include <wex/config.h>
#include <wex/core.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/tokenizer.h>
#include <wx/app.h>

#include "config-imp.h"

using json = nlohmann::json;

wex::config_imp::config_imp()
  : m_json({})
{
}

wex::config_imp::config_imp(const config_imp* c, const std::string& item)
  : m_json(c == nullptr ? json({}) : c->m_json)
  , m_item(item)
{
  if (!item.empty())
  {
    if (const auto& it = m_json.find(item); it != m_json.end())
    {
      m_json = *it;
    }
    else
    {
      m_json.clear();
    }
  }
}

json& wex::config_imp::accessor(const std::string& item)
{
  switch (const auto& v(tokenize<std::vector<std::string>>(item, "."));
          v.size())
  {
    case 0:
      return m_json;

    case 1:
      return m_json[v[0]];

    case 2:
      return m_json[v[0]][v[1]];

    case 3:
      return m_json[v[0]][v[1]][v[2]];

    case 4:
      return m_json[v[0]][v[1]][v[2]][v[3]];

    case 5:
      return m_json[v[0]][v[1]][v[2]][v[3]][v[4]];

    default:
      log("too deeply nested") << v.size() << "hierarchy for:" << item;
      return m_json;
  }
}

size_t wex::config_imp::elements() const
{
  size_t total = 0;
  elements(m_json, total);
  return total;
}

void wex::config_imp::elements(const json& o, size_t& total) const
{
  total += o.size();

  for (const auto& x : o.items())
  {
    if (x.value().type() == json::value_t::object)
    {
      elements(x.value(), total);
    }
  }
}

bool wex::config_imp::exists(const std::string& item)
{
  if (item.find('.') == std::string::npos)
  {
    return m_json.contains(item);
  }
  else
  {
    json& access(accessor(before(item, '.', false)));
    return !access.is_null() && access.contains(after(item, '.', false));
  }
}

void wex::config_imp::read()
{
  if (m_file.empty())
  {
    m_file =
      path(config::dir(), wxTheApp->GetAppName().Lower() + ".json").string();
  }

  if (std::ifstream fs(m_file); fs.is_open())
  {
    try
    {
      fs >> m_json;
    }
    catch (std::exception& e)
    {
      log(e) << m_file;
    }
  }
  else
  {
    log::debug("could not read") << m_file;
  }
}

void wex::config_imp::save() const
{
  if (std::ofstream fs(m_file); fs.is_open())
  {
    fs << std::setw(2) << m_json << std::endl;
  }
  else
  {
    log("could not save") << m_file;
  }
}
