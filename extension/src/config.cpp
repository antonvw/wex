////////////////////////////////////////////////////////////////////////////////
// Name:      config.cpp
// Purpose:   Implementation of class wex::config
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <numeric>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/app.h>
#include <wx/colour.h>
#include <wx/fileconf.h> 
#include <wx/config.h>
#include <wx/font.h>
#include <wx/stdpaths.h>
#include <wex/config.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

wex::config::config(type type)
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
    wxGetHomeDir().ToStdString(), 
    ".config", 
    wxTheApp->GetAppName().Lower().ToStdString()}).Path().string();
#endif
}

bool wex::config::empty() const
{
  return get().empty();
}
  
void wex::config::erase()
{
  wxConfigBase::Get()->DeleteEntry(m_item);
  wxConfigBase::Get()->DeleteGroup(m_item);
}
  
bool wex::config::exists() const
{
  return wxConfigBase::Get()->Exists(m_item);
}
  
const std::string wex::config::firstof() const
{
  return 
    wxConfigBase::Get()->Read(m_item).BeforeFirst(
      get_field_separator()).ToStdString();
}

const std::string wex::config::firstof_write(const std::string& value) const
{
  std::vector<std::string> v{value};

  for (tokenizer tkz(wxConfigBase::Get()->Read(m_item).ToStdString(), 
    std::string(1, get_field_separator())); tkz.HasMoreTokens(); )
  {
    if (const std::string val = tkz.GetNextToken(); val != value)
    {
      v.emplace_back(val);
    }
  }

  wxConfigBase::Get()->Write(m_item, 
    std::accumulate(v.begin(), v.end(), wxString{}, 
    [&](const wxString& a, const wxString& b) {
      return a + b + get_field_separator();}));
  
  return value;
}
  
const std::string wex::config::get(const std::string& def) const
{
  return wxConfigBase::Get()->Read(m_item, def).ToStdString();
}

const std::string wex::config::get(const char* def) const
{
  return get(std::string(def));
}
  
bool wex::config::get(bool def) const
{
  return wxConfigBase::Get()->ReadBool(m_item, def);
}

long wex::config::get(long def) const
{
  return wxConfigBase::Get()->ReadLong(m_item, def);
}

int wex::config::get(int def) const
{
  return wxConfigBase::Get()->ReadLong(m_item, def);
}

float wex::config::get(float def) const
{
  return wxConfigBase::Get()->ReadDouble(m_item, def);
}

double wex::config::get(double def) const
{
  return wxConfigBase::Get()->ReadDouble(m_item, def);
}

wxColour wex::config::get(const wxColour& def) const
{
  return wxConfigBase::Get()->ReadObject(m_item, def);
}
    
wxFont wex::config::get(const wxFont& def) const
{
  return wxConfigBase::Get()->ReadObject(m_item, def);
}
    
const std::list < std::string > wex::config::get_list() const
{
  return tokenizer(
    get(std::string()), 
    std::string(1, get_field_separator())).Tokenize<
      std::list < std::string >>();
}

void wex::config::init()
{
  wxConfigBase::Set(new wxFileConfig(
    wxEmptyString, 
    wxEmptyString,
    wxFileName(dir(), wxTheApp->GetAppName().Lower() + 
#ifdef __WXMSW__
    ".ini"
#else
    ".conf"
#endif
      ).GetFullPath(), wxEmptyString, wxCONFIG_USE_LOCAL_FILE));
}

void wex::config::set(const std::string& def)
{
  wxConfigBase::Get()->Write(m_item, def.c_str());
}

void wex::config::set(const char* def)
{
  wxConfigBase::Get()->Write(m_item, def);
}

void wex::config::set(bool def)
{
  wxConfigBase::Get()->Write(m_item, def);
}

void wex::config::set(long def)
{
  wxConfigBase::Get()->Write(m_item, def);
}

void wex::config::set(int def)
{
  wxConfigBase::Get()->Write(m_item, def);
}

void wex::config::set(float def)
{
  wxConfigBase::Get()->Write(m_item, def);
}

void wex::config::set(double def)
{
  wxConfigBase::Get()->Write(m_item, def);
}

void wex::config::set(const wxColour& def)
{
  wxConfigBase::Get()->Write(m_item, def);
}
  
void wex::config::set(const wxFont& def)
{
  wxConfigBase::Get()->Write(m_item, def);
}
  
void wex::config::set(const std::list < std::string > & l)
{
  if (l.empty()) return;

  std::string text;
  const int commandsSaveInConfig = 75;
  int items = 0;

  for (const auto& it : l)
  {
    if (items++ > commandsSaveInConfig) break;
    text += it + get_field_separator();
  }
  
  set(text);
}

void wex::config::set_record_defaults(bool val)
{
  wxConfigBase::Get()->SetRecordDefaults(val);
}

wxConfigBase* wex::config::wx_config()
{
  return wxConfigBase::Get();
}
