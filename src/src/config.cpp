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
#include <wex/tokenizer.h>
#include <wex/util.h>

using json = nlohmann::json;

namespace wex
{
  class config_imp
  {
  public:
    /// Static iinterface.

    /// Sets config file to use. If not called,
    /// the default is used.
    static void set_file(const std::string& file)
    {
      m_file = file;
    }
    
    /// Other methods.
    
    /// Default constructor.
    config_imp() : m_json({}) {;};
    
    /// Copy constructor with a possible child item.
    config_imp(
      const config_imp* c, 
      const std::string& item = std::string())
      : m_json(c == nullptr ? json({}): c->m_json)
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
    
    /// Returns true if this item is present.
    bool exists(const std::string& item) 
    {
      if (item.find('.') == std::string::npos)
      {
        return m_json.contains(item);
      }
      else
      {
        json& access (accessor(before(item, '.', false)));
        return !access.is_null() && access.contains(after(item, '.', false));
      }
    }
    
    /// Returns item.
    auto & get_item() const {return m_item;};

    /// Returns json.
    const json& get_json() const {return m_json;};
    json& get_json() {return m_json;};

    /// Reads json file.
    void read()
    {
      if (m_file.empty())
      {
        m_file = path(
          config::dir(), 
          wxTheApp->GetAppName().Lower() + ".json").string();
      }

      if (std::ifstream fs(m_file); fs.is_open())
      {
        fs >> m_json;
      }
      else
      {
        log::verbose("could not read") << m_file;
      }
    }

    /// Saves json file.
    void save() const
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
    
    /// Sets value for item.
    template <typename T>
    void set(const std::string& item, const T& v)
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
    
    /// Returns value for item.
    template <typename T>
    const T value(const std::string& item, const T& def)
    {
      if (item.find('.') == std::string::npos)
      {
        return m_json.value(item, def);
      }
      else
      {
        auto& a(accessor(before(item, '.', false)));
        return !a.is_null() ?
          a.value(after(item, '.', false), def):
          def;
      }
    }
  private:
    json& accessor(const std::string& item)
    {
      const auto& v(
        tokenizer(item, ".").tokenize<std::vector<std::string>>());

      switch (v.size())
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

    json m_json;
    static inline std::string m_file;
    const std::string m_item;
  };
};

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
  m_store->get_json()[m_local->get_item()] = m_local->get_json();
  
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
  return m_local == nullptr ? 0: m_local->get_json().size();
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
  return !m_item.empty() ? get_store()->exists(m_item): false;
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
  
wex::config& wex::config::item(const std::string& item) 
{
  m_item = item; 

  return *this;
}

void wex::config::on_exit()
{
  m_store->save();
  delete m_store;
}

void wex::config::on_init()
{
  m_store = new config_imp();
  m_store->read();
  
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
  
void wex::config::set_file(const std::string& file)
{
  config_imp::set_file(file);
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
