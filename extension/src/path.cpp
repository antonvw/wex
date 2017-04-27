////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wxExPath
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/filename.h>
#include <wx/extension/path.h>
#include <wx/extension/lexers.h>

class wxExPathImp : public wxFileName
{
public:
  wxExPathImp(const std::string& fullpath) 
    : wxFileName(fullpath) {;};
};
  
wxExPath::wxExPath(const std::string& fullpath)
  : m_Path(std::make_unique<wxExPathImp>(fullpath))
  , m_Stat(fullpath) 
  , m_Lexer(wxExLexers::Get(false) != nullptr ? 
      wxExLexers::Get(false)->FindByFileName(
        wxFileName(fullpath).GetFullName().ToStdString()):
      std::string())
{
}

wxExPath::wxExPath(const std::string& path, const std::string& name)
  : wxExPath(std::string(path + wxFileName::GetPathSeparator() + name))
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

bool wxExPath::DirExists() const 
{
  return m_Path->DirExists();
}

bool wxExPath::FileExists() const
{
  return m_Path->FileExists();
}

const std::string wxExPath::GetExtension() const 
{
  return m_Path->GetExt().ToStdString();
}

const std::string wxExPath::GetFullName() const 
{
  return m_Path->GetFullName().ToStdString();
}

const std::string wxExPath::GetFullPath() const 
{
  return m_Path->GetFullPath().ToStdString();
}

const std::string wxExPath::GetName() const 
{
  return m_Path->GetName().ToStdString();
}

const std::string wxExPath::GetPath() const 
{
  return m_Path->GetPath().ToStdString();
}

const std::string wxExPath::GetVolume() const 
{
  return m_Path->GetVolume().ToStdString();
}

bool wxExPath::IsAbsolute() const
{
  return m_Path->IsAbsolute();
}
  
bool wxExPath::IsReadOnly() const 
{
  return m_Stat.IsReadOnly();
}
    
bool wxExPath::IsRelative() const
{
  return m_Path->IsRelative();
}
  
bool wxExPath::MakeAbsolute() 
{
  return m_Path->MakeAbsolute();
}

bool wxExPath::Normalize(const std::string& cwd)
{
  return m_Path->Normalize(wxPATH_NORM_ALL, cwd);
}
