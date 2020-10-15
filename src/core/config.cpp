////////////////////////////////////////////////////////////////////////////////
// Name:      config.cpp
// Purpose:   Implementation of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/config.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wx/app.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/stdpaths.h>

#include "config-imp.h"

wex::config::config(const std::string& item)
{
  config::item(item);
}

wex::config::config(const std::string& parent, const std::string& child)
  : m_item(child)
  , m_local(new config_imp(m_store, parent))
{
}

wex::config::~config()
{
  child_end();
}

bool wex::config::change_file(const std::string& file)
{
  if (!child_end())
  {
    return false;
  }

  m_store->save();

  config_imp::set_file(file);

  m_store->read();

  return true;
}

bool wex::config::child_end()
{
  if (m_local == nullptr || m_store == nullptr)
  {
    return false;
  }

  // copy local store to shared store to add / set item
  try
  {
    m_store->get_json()[m_local->get_item()] = m_local->get_json();
  }
  catch (std::exception& e)
  {
    log(e) << "child_end" << m_item;
  }

  // cleanup local store
  delete m_local;
  m_local = nullptr;

  return true;
}

bool wex::config::child_start()
{
  if (m_local != nullptr || m_item.empty())
  {
    return false;
  }

  m_local = new config_imp(m_store, m_item);

  return true;
}

size_t wex::config::children() const
{
  return m_local == nullptr ? 0 : m_local->get_json().size();
}

const std::string wex::config::dir()
{
  if (const path p({wxGetHomeDir(), ".config", wxTheApp->GetAppName().Lower()});
      p.dir_exists())
  {
    return p.data().string();
  }
  else if (const path p({wxGetHomeDir(), ".config", "wex"}); p.dir_exists())
  {
    return p.data().string();
  }
  else
  {
    return wxPathOnly(wxStandardPaths::Get().GetExecutablePath()).ToStdString();
  }
}

bool wex::config::empty() const
{
  return get().empty();
}

void wex::config::erase() const
{
  if (!m_item.empty())
  {
    get_store()->get_json().erase(m_item);
  }
}

bool wex::config::exists() const
{
  return !m_item.empty() ? get_store()->exists(m_item) : false;
}

const std::string wex::config::file()
{
  return config_imp::file();
}

const std::string wex::config::get(const char* def) const
{
  return get(std::string(def));
}

const std::string wex::config::get(const std::string& def) const
{
  return get_store()->value(m_item, def);
}

bool wex::config::get(bool def) const
{
  return get_store()->value(m_item, def);
}

long wex::config::get(long def) const
{
  return get_store()->value(m_item, def);
}

int wex::config::get(int def) const
{
  return get_store()->value(m_item, def);
}

float wex::config::get(float def) const
{
  return get_store()->value(m_item, def);
}

double wex::config::get(double def) const
{
  return get_store()->value(m_item, def);
}

const std::list<std::string>
wex::config::get(const std::list<std::string>& def) const
{
  return get_store()->value(m_item, def);
}

const std::vector<int> wex::config::get(const std::vector<int>& def) const
{
  return get_store()->value(m_item, def);
}

const wex::config::statusbar_t wex::config::get(const statusbar_t& def) const
{
  statusbar_t s;

  for (const auto& it : def)
  {
    const auto style = get_store()->value(
      m_item + ".styles." + std::get<0>(it),
      std::get<1>(it));

    const int width = get_store()->value(
      m_item + ".widths." + std::get<0>(it),
      std::get<2>(it));

    s.push_back({std::get<0>(it), style, width});
  }

  return s;
}

wxColour wex::config::get(const wxColour& def) const
{
  wxColour col;
  wxFromString(get(wxToString(def).ToStdString()), &col);
  return col;
}

wxFont wex::config::get(const wxFont& def) const
{
  wxFont wx;
  wxFromString(get(wxToString(def).ToStdString()), &wx);
  return wx;
}

const std::string wex::config::get_firstof() const
{
  const auto& l(get(std::list<std::string>{}));
  return l.empty() ? std::string() : l.front();
}

wex::config_imp* wex::config::get_store() const
{
  if (m_item.find(".") != std::string::npos)
  {
    return m_store;
  }

  return m_local != nullptr ? m_local : m_store;
}

bool wex::config::is_child() const
{
  return m_local != nullptr;
}

wex::config& wex::config::item(const std::string& item)
{
  m_item = item;

  return *this;
}

void wex::config::on_exit()
{
  if (m_store_save)
  {
    m_store->save();
  }

  delete m_store;
}

void wex::config::on_init()
{
  m_store = new config_imp();
  m_store->read();

  log::verbose("config") << "top size:" << size()
                         << "elements:" << m_store->elements();
}

void wex::config::read()
{
  m_store->read();
}

void wex::config::save()
{
  m_store->save();
}

void wex::config::set(const std::string& v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const char* v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(bool v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(long v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(int v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(float v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(double v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const std::list<std::string>& v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const std::vector<int>& v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const statusbar_t& v)
{
  for (const auto& it : v)
  {
    get_store()->set(m_item + ".styles." + std::get<0>(it), std::get<1>(it));
    get_store()->set(m_item + ".widths." + std::get<0>(it), std::get<2>(it));
  }
}

void wex::config::set(const wxColour& v)
{
  get_store()->set(m_item, wxToString(v).ToStdString());
}

void wex::config::set(const wxFont& v)
{
  get_store()->set(m_item, wxToString(v).ToStdString());
}

void wex::config::set_file(const std::string& file)
{
  config_imp::set_file(file);
}

const std::string wex::config::set_firstof(const std::string& v, size_t max)
{
  auto l(get(std::list<std::string>{}));

  l.remove(v);
  l.push_front(v);

  if (l.size() > max)
  {
    l.remove(l.back());
  }

  set(l);

  return v;
}

size_t wex::config::size()
{
  return m_store->get_json().size();
}

bool wex::config::toggle(bool def)
{
  get_store()->set(m_item, !get_store()->value(m_item, def));
  return get_store()->value(m_item, def);
}
