////////////////////////////////////////////////////////////////////////////////
// Name:      filename.cpp
// Purpose:   Implementation of class wxExFileName
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/filename.h>
#include <wx/extension/lexers.h>

wxExFileName::wxExFileName(const std::string& fullpath)
  : m_FileName(fullpath)
  , m_Stat(fullpath) 
  , m_Lexer(wxExLexers::Get(false) != nullptr ? 
      wxExLexers::Get(false)->FindByFileName(wxFileName(fullpath).GetFullName().ToStdString()):
      std::string())
{
}

wxExFileName::wxExFileName(const std::string& path, const std::string& name)
  : wxExFileName(std::string(path + wxFileName::GetPathSeparator() + name))
{
}

wxExFileName::wxExFileName(const char* fullpath)
  : wxExFileName(std::string(fullpath)) 
{
}

wxExFileName::wxExFileName(const wxFileName& filename)
  : wxExFileName(filename.GetFullPath().ToStdString()) 
{
}
