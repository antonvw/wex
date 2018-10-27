////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/mimetype.h>
#include <wx/url.h>
#include <wex/path.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/util.h>

namespace fs = std::filesystem;

const std::string SubstituteTilde(const std::string& text)
{
  auto out(text);
  wex::replace_all(out, "~", wxGetHomeDir().ToStdString());
  return out;
}

wex::path::path(const fs::path& p)
  : m_path(p)
  , m_Stat(p.string()) 
  , m_Lexer(lexers::Get(false) != nullptr ? 
      lexers::Get(false)->FindByFileName(p.filename().string()):
      std::string())
{
  if (p.empty())
  {
    m_path_original = Current();
  }
}

wex::path::path(const std::string& p, const std::string& name)
  : path(fs::path(SubstituteTilde(p)).append(name).string())
{
}

wex::path::path(const std::string& p)
  : path(fs::path(SubstituteTilde(p)))
{
}

wex::path::path(const char* p)
  : path(fs::path(p))
{
}

wex::path::path(const path& r)
  : path(r.Path()) 
{
}

wex::path::path(const std::vector<std::string> & v)
  : path() 
{
  for (const auto& it : v)
  {
    Append(it);
  }
}

wex::path::~path()
{
  if (!m_path_original.empty())
  {
    Current(m_path_original);
  }
}

wex::path& wex::path::operator=(const wex::path& r)
{
  if (this != &r)
  {
    m_path = r.Path();
    m_Lexer = r.m_Lexer;
    m_Stat = r.m_Stat;
  }
  
  return *this;
}

wex::path& wex::path::Append(const wex::path& path)
{
  m_path /= fs::path(path.Path());

  return *this;
}

void wex::path::Current(const std::string& path) 
{
  if (!path.empty())
  {
    try
    {
      fs::current_path(path);
    }
    catch (const std::exception& e)
    {
      log(e) << "path:" << path;
    }
  }
}

const std::vector<wex::path> wex::path::paths() const
{
  std::vector<path> v;

  for (const auto& e : m_path)
  {
    v.emplace_back(e);
  }

  return v;
}

wex::path& wex::path::make_absolute() 
{
  m_path = fs::absolute(m_path);
  m_Stat.sync();

  if (!fs::is_directory(m_path.parent_path())) 
  {
    m_path.clear();
  }

  return *this;
}

bool wex::path::open_mime() const
{
  if (const auto & ex = GetExtension(); ex.empty())
  {
    if (wxURL(m_path.string()).IsOk() || 
        m_path.string().substr(0, 4) == "http")
    {
      return wxLaunchDefaultBrowser(m_path.string());
    }
    else
    {
      return false;
    }
  }
  else if (auto* type = wxTheMimeTypesManager->GetFileTypeFromExtension(ex);
    type == nullptr)
  {
    return false;
  }
  else if (const auto command(type->GetOpenCommand(Path().string())); command.empty())
  {
    return false;
  }
  else
  {
    return wxExecute(command) != -1;
  }
}

wex::path& wex::path::replace_filename(const std::string& filename)
{
  m_path.replace_filename(filename);

  return *this;
}
