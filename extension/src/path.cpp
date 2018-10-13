////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/mimetype.h>
#include <wx/url.h>
#include <wx/extension/path.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>

const std::string SubstituteTilde(const std::string& text)
{
  auto out(text);
  wex::replace_all(out, "~", wxGetHomeDir().ToStdString());
  return out;
}

wex::path::path(const std::experimental::filesystem::path& p)
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

wex::path::path(const std::string& path, const std::string& name)
  : wex::path(std::experimental::filesystem::path(SubstituteTilde(path)).
      append(name).string())
{
}

wex::path::path(const std::string& path)
  : wex::path(std::experimental::filesystem::path(SubstituteTilde(path)))
{
}

wex::path::path(const char* path)
  : wex::path(std::experimental::filesystem::path(path))
{
}

wex::path::path(const path& r)
  : path(r.Path()) 
{
}

wex::path::path(const std::vector<std::string> v)
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
  m_path /= std::experimental::filesystem::path(path.Path());

  return *this;
}

bool wex::path::Canonical(const std::string& wd)
{
  if (!std::experimental::filesystem::is_directory(wd) || 
      !std::experimental::filesystem::is_regular_file(wd))
  {
    return false;
  }

  m_path = std::experimental::filesystem::canonical(m_path, wd);

  return true;
}

void wex::path::Current(const std::string& path) 
{
  if (!path.empty())
  {
    try
    {
      std::experimental::filesystem::current_path(path);
    }
    catch (const std::exception& e)
    {
      log(e) << "path:" << path;
    }
  }
}

const std::vector<wex::path> wex::path::GetPaths() const
{
  std::vector<path> v;

  for (const auto& e : m_path)
  {
    v.emplace_back(e);
  }

  return v;
}

wex::path& wex::path::MakeAbsolute(const path& base) 
{
  m_path = std::experimental::filesystem::absolute(m_path, base.Path().empty() ? 
    std::experimental::filesystem::current_path(): 
    std::experimental::filesystem::path(base.GetPath()));

  m_Stat.Sync();

  if (!std::experimental::filesystem::is_directory(m_path.parent_path())) 
  {
    m_path.clear();
  }

  return *this;
}

bool wex::path::OpenMIME() const
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

wex::path& wex::path::ReplaceFileName(const std::string& filename)
{
  m_path.replace_filename(filename);

  return *this;
}
