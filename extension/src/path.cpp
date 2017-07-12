////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wxExPath
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/path.h>
#include <wx/extension/lexers.h>

wxExPath::wxExPath(const std::experimental::filesystem::path& p)
  : m_path(p)
  , m_Stat(p.string()) 
  , m_Lexer(wxExLexers::Get(false) != nullptr ? 
      wxExLexers::Get(false)->FindByFileName(p.filename().string()):
      std::string())
{
}

wxExPath::wxExPath(const std::string& path, const std::string& name)
  : wxExPath(std::experimental::filesystem::path(path).append(name).string())
{
}

wxExPath::wxExPath(const std::string& path)
  : wxExPath(std::experimental::filesystem::path(path))
{
}

wxExPath::wxExPath(const char* path)
  : wxExPath(std::experimental::filesystem::path(path))
{
}

wxExPath::wxExPath(const wxExPath& r)
  : wxExPath(r.Path()) 
{
}

wxExPath::wxExPath(const std::vector<std::string> v)
  : wxExPath() 
{
  for (const auto& it : v)
  {
    Append(it);
  }
}

wxExPath& wxExPath::operator=(const wxExPath& r)
{
  if (this != &r)
  {
    m_path = r.Path();
    m_Lexer = r.m_Lexer;
    m_Stat = r.m_Stat;
  }
  
  return *this;
}

wxExPath& wxExPath::Append(const wxExPath& path)
{
  m_path /= std::experimental::filesystem::path(path.Path());

  return *this;
}

bool wxExPath::Canonical(const std::string& wd)
{
  if (!std::experimental::filesystem::is_directory(wd) || 
      !std::experimental::filesystem::is_regular_file(wd))
  {
    return false;
  }

  m_path = std::experimental::filesystem::canonical(m_path, wd);

  return true;
}

const std::vector<wxExPath> wxExPath::GetPaths() const
{
  std::vector<wxExPath> v;

  for (const auto& e : m_path)
  {
    v.emplace_back(e.string());
  }

  return v;
}

wxExPath& wxExPath::MakeAbsolute(const wxExPath& base) 
{
  m_path = std::experimental::filesystem::absolute(m_path, base.Path().empty() ? 
    std::experimental::filesystem::current_path(): 
    std::experimental::filesystem::path(base.GetPath()));

  if (!std::experimental::filesystem::is_directory(m_path.parent_path())) 
  {
    m_path.clear();
  }

  return *this;
}

wxExPath& wxExPath::ReplaceFileName(const std::string& filename)
{
  m_path.replace_filename(filename);

  return *this;
}
