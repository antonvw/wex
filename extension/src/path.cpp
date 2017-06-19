////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wxExPath
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <experimental/filesystem>
#include <wx/extension/path.h>
#include <wx/extension/lexers.h>

class wxExPathImp
{
public:
  wxExPathImp(const std::string& fullpath = std::string()) 
    : m_path(fullpath) {;};

  void Absolute(const wxExPath& base) { 
    m_path = std::experimental::filesystem::absolute(m_path,
      base.GetFullPath().empty() ? 
        std::experimental::filesystem::current_path(): 
        std::experimental::filesystem::path(base.GetPath()));

    if (!std::experimental::filesystem::is_directory(m_path.parent_path())) 
    {
      m_path.clear();
    }};

  void Append(const std::string& path) {
    m_path /= std::experimental::filesystem::path(path);};

  bool Canonical(const std::string& wd) { 
    if (!std::experimental::filesystem::is_directory(wd) || 
        !std::experimental::filesystem::is_directory(wd))
    {
      return false;
    }
    m_path = std::experimental::filesystem::canonical(m_path, wd);
    return true;};

  const auto& Path() {return m_path;};

  void ReplaceFileName(const std::string& filename) {
    m_path.replace_filename(filename);}
private:
  std::experimental::filesystem::path m_path;
};
  
wxExPath::wxExPath(const std::string& fullpath)
  : m_Path(std::make_unique<wxExPathImp>(fullpath))
  , m_Stat(fullpath) 
  , m_Lexer(wxExLexers::Get(false) != nullptr ? 
      wxExLexers::Get(false)->FindByFileName(
        std::experimental::filesystem::path(fullpath).filename()):
      std::string())
{
}

wxExPath::wxExPath(const std::string& path, const std::string& name)
  : wxExPath(std::experimental::filesystem::path(path) /= name)
{
}

wxExPath::wxExPath(const char* fullpath)
  : wxExPath(std::string(fullpath)) 
{
}

wxExPath::wxExPath(const wxExPath& r)
  : wxExPath(r.GetFullPath()) 
{
}

wxExPath::wxExPath(const std::vector<std::string> v)
  : wxExPath() 
{
  for (const auto& it : v)
  {
    m_Path->Append(it);
  }
}

wxExPath::~wxExPath()
{
}
  
wxExPath& wxExPath::operator=(const wxExPath& r)
{
  if (this != &r)
  {
    m_Path = std::make_unique<wxExPathImp>(r.GetFullPath());
    m_Lexer = r.m_Lexer;
    m_Stat = r.m_Stat;
  }
  
  return *this;
}

wxExPath& wxExPath::Append(const wxExPath& path)
{
  m_Path->Append(path.GetFullPath());

  return *this;
}

bool wxExPath::Canonical(const std::string& cwd)
{
  return m_Path->Canonical(cwd);
}

bool wxExPath::DirExists() const 
{
  return std::experimental::filesystem::is_directory(m_Path->Path());
}

bool wxExPath::FileExists() const
{
  return std::experimental::filesystem::is_regular_file(m_Path->Path());
}

const std::string wxExPath::GetExtension() const 
{
  return m_Path->Path().extension().string();
}

const std::string wxExPath::GetFullName() const 
{
  return m_Path->Path().filename().string();
}

const std::string wxExPath::GetFullPath() const 
{
  return m_Path->Path().string();
}

const std::string wxExPath::GetName() const 
{
  return m_Path->Path().stem().string();
}

const std::string wxExPath::GetPath() const 
{
  return m_Path->Path().parent_path().string();
}

const std::vector<wxExPath> wxExPath::GetPaths() const
{
  std::vector<wxExPath> v;

  for (const auto& e : m_Path->Path())
  {
    v.emplace_back(e);
  }

  return v;
}

const std::string wxExPath::GetVolume() const 
{
  return m_Path->Path().root_name().string();
}

bool wxExPath::IsAbsolute() const
{
  return m_Path->Path().is_absolute();
}
  
bool wxExPath::IsReadOnly() const 
{
  return m_Stat.IsReadOnly();
}
    
bool wxExPath::IsRelative() const
{
  return m_Path->Path().is_relative();
}
  
wxExPath& wxExPath::MakeAbsolute(const wxExPath& base) 
{
  m_Path->Absolute(base);

  return *this;
}

wxExPath& wxExPath::ReplaceFileName(const std::string& filename)
{
  m_Path->ReplaceFileName(filename);

  return *this;
}
