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

using json = nlohmann::json;

namespace wex
{
  class config_imp
  {
  public:
    explicit config_imp(const std::string& dir)
      : m_path(
          dir, 
          wxTheApp->GetAppName().Lower() + ".json")
      , m_json({})
    {
      if (std::ifstream fs(m_path.data());
        fs.is_open())
      {
        fs >> m_json;
      }
    }
    
   ~config_imp() 
    {
      if (std::ofstream fs(m_path.data());
        fs.is_open())
      {
        fs << std::setw(2) << m_json << std::endl;
      }
    };

    const json& get_json() const {return m_json;};
    json& get_json() {return m_json;};
  private:
    json m_json;
    const path m_path;
  };
};

wex::config::config(type_t type)
  : m_type(type)
{
}
  
wex::config::config(const std::string& item)
  : m_item(item)
{
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
  
void wex::config::erase()
{
  if (!m_item.empty())
  {
    m_store->get_json().erase(m_item);
  }
}
  
bool wex::config::exists() const
{
  return !m_item.empty() ? m_store->get_json().contains(m_item): false;
}
  
const std::string wex::config::get(const char* def) const
{
  return get(std::string(def));
}
  
#define READ \
  return m_store->get_json().value(m_item, def);
  
const std::string wex::config::get(const std::string& def) const {READ;}
bool wex::config::get(bool def) const {READ;}
long wex::config::get(long def) const {READ;}
int wex::config::get(int def) const {READ;}
float wex::config::get(float def) const {READ;}
double wex::config::get(double def) const {READ;}
const std::list < std::string > wex::config::get(
  const std::list<std::string>& def) const {READ;}
const std::vector < int > wex::config::get(
  const std::vector < int > & def) const {READ;};
const std::vector < std::tuple < std::string, int, int > > wex::config::get(
  const std::vector < std::tuple < std::string, int, int > > & def) const {READ;};

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

void wex::config::init()
{
  m_store = new config_imp(dir());
}

void wex::config::on_exit()
{
  delete m_store;
}

#define WRITE                           \
  if (!m_item.empty())                  \
  {                                     \
    m_store->get_json()[m_item] = v;  \
  }
  
void wex::config::set(const std::string& v) {WRITE;}
void wex::config::set(const char* v) {WRITE;}
void wex::config::set(bool v) {WRITE;}
void wex::config::set(long v) {WRITE;}
void wex::config::set(int v) {WRITE;}
void wex::config::set(float v) {WRITE;}
void wex::config::set(double v) {WRITE;}
void wex::config::set(const std::list < std::string > & v) {WRITE;}
void wex::config::set(const std::vector < int > & v) {WRITE;}
void wex::config::set(
  const std::vector < std::tuple < std::string, int, int > > & v) {WRITE;}

void wex::config::set(const wxColour& v) 
{
  m_store->get_json()[m_item] = wxToString(v).ToStdString();
}

void wex::config::set(const wxFont& v) 
{
  m_store->get_json()[m_item] = wxToString(v).ToStdString();
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
