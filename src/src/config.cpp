////////////////////////////////////////////////////////////////////////////////
// Name:      config.cpp
// Purpose:   Implementation of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <fstream>
#include <iomanip> 
#include <numeric>
#include <nlohmann/json.hpp>

#include <wx/app.h>
#include <wx/colour.h>
#include <wx/font.h>
#include <wx/stdpaths.h>
#include <wex/config.h>
#include <wex/path.h>
#include <wex/util.h>

using json = nlohmann::json;

namespace wex
{
  class config_imp
  {
  public:
    /// Constructor, provide directory for json file.
    explicit config_imp(const std::string& dir)
      : m_json({})
      , m_path(
          dir, 
          wxTheApp->GetAppName().Lower() + ".json")
    {
      if (std::ifstream fs(m_path.data());
        fs.is_open())
      {
        fs >> m_json;
      }
    }
    
    /// Copy constructor with filter.
    config_imp(const config_imp& c, const std::string& filter)
      : m_json(c.m_json)
      , m_key(filter)
    {
      log::verbose("json size") << m_json.size() << "filter" << filter;

      const auto& it = m_json.find(filter);
      
      if (it != m_json.end())
      {
        m_json = *it;
        log::verbose("json set") << m_json.dump();
      }
      else
      {
        m_json.clear();
        log::verbose("json cleared") << filter;
      }
    }
    
    /// Destructor.
   ~config_imp() 
    {
      // store only if this is not a child
      if (m_key.empty())
      {
        if (std::ofstream fs(m_path.data());
          fs.is_open())
        {
          fs << std::setw(2) << m_json << std::endl;
        }
      }
    };

    /// Returns json.
    const json& get_json() const {return m_json;};
    json& get_json() {return m_json;};

    /// Returns key.
    auto & get_key() {return m_key;};

    /// Sets value for item.
    template <typename T>
    void set(const std::string& item, const T& v)
    {
      if (!item.empty())
      {
        m_json[item] = v;
      }
    }
    
    /// Returns value for item.
    template <typename T>
    const T value(const std::string& item, const T& def) const
    {
      if (item.find(".") != std::string::npos)
      {
        const auto& it = m_json.find(before(item, '.'));

        if (it != m_json.end())
        {
          json child = *it;
          return child.value(after(item, '.'), def);
        }
        else
        {
          return def;
        }
      }
      else
      {
        return m_json.value(item, def);
      }
    }
  private:
    json m_json;
    const std::string m_key;
    const path m_path;
  };
};

wex::config::config(const std::string& item)
  : m_item(item)
{
}
  
wex::config::config(const std::string& parent, const std::string& child)
  : m_item(child)
  , m_local(new config_imp(*m_store, parent))
{
}
  
wex::config::~config()
{
  child_end();
}

bool wex::config::child_end()
{
  if (m_local == nullptr)
  {
    return false;
  }

  // copy local store to shared store to add / set item
  m_store->get_json()[m_local->get_key()] = m_local->get_json();
  
  log::verbose("config child end") 
    << "key:" << m_local->get_key() 
    << "item:" << m_item
    << "children:" << children()
    << "size:" << size()
    << m_local->get_json().dump();
  
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
  
  m_local = new config_imp(*m_store, m_item);
  
  return true;
}
  
size_t wex::config::children() const
{
  return m_local->get_json().size();
}
  
const std::string wex::config::dir()
{
#ifdef __WXMSW__
  return wxPathOnly(wxStandardPaths::Get().
    GetExecutablePath()).ToStdString();
#else
  return path({
    wxGetHomeDir(), 
    ".config", 
    wxTheApp->GetAppName().Lower()}).data().string();
#endif
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
  return !m_item.empty() ? get_store()->get_json().contains(m_item): false;
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

const std::list < std::string > wex::config::get(
  const std::list<std::string>& def) const
{
  return get_store()->value(m_item, def);
}

const std::vector < int > wex::config::get(
  const std::vector < int > & def) const
{
  return get_store()->value(m_item, def);
}

const wex::config::statusbar_t wex::config::get(
  const statusbar_t & def) const
{
  return get_store()->value(m_item, def);
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
  const auto& l (get(std::list<std::string>{}));
  return l.empty() ? std::string(): l.front();
}

wex::config_imp* wex::config::get_store() const 
{
  if (m_item.find(".") != std::string::npos)
  {
    return m_store;
  }

  return m_local != nullptr ? m_local: m_store;
}
  
bool wex::config::is_child() const
{
  return m_local != nullptr;
}
  
void wex::config::on_exit()
{
  delete m_store;
}

void wex::config::on_init()
{
  m_store = new config_imp(dir());
  
  log::verbose("config") << "size:" << size();
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

void wex::config::set(const std::list < std::string > & v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const std::vector < int > & v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const statusbar_t & v)
{
  get_store()->set(m_item, v);
}

void wex::config::set(const wxColour& v) 
{
  get_store()->set(m_item, wxToString(v).ToStdString());
}

void wex::config::set(const wxFont& v) 
{
  get_store()->set(m_item, wxToString(v).ToStdString());
}
  
const std::string wex::config::set_firstof(const std::string& v, size_t max)
{
  auto l (get(std::list<std::string>{}));
    
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
