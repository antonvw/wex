////////////////////////////////////////////////////////////////////////////////
// Name:      filename.cpp
// Purpose:   Implementation of class 'wxExFileName'
// Author:    Anton van Wezenbeek
// Created:   2010-03-18
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stdpaths.h> // strangely enough, for wxTheFileIconsTable
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/extension/filename.h>
#include <wx/extension/lexers.h>

wxExFileName::wxExFileName(const wxString& fullpath, wxPathFormat format)
  : wxFileName(fullpath, format)
  , m_Stat(fullpath) 
{
  // This construct prevents an assert in the test base.
  wxExLexers* lexers = wxExLexers::Get(false);

  if (lexers != NULL)
  {
    m_Lexer = lexers->FindByFileName(*this);
  }
}

wxExFileName::wxExFileName(const wxFileName& filename)
  : wxFileName(filename)
  , m_Stat(filename.GetFullPath()) 
{
  wxExLexers* lexers = wxExLexers::Get(false);

  if (lexers != NULL)
  {
    m_Lexer = lexers->FindByFileName(*this);
  }
}

int wxExFileName::GetIconID() const
{
  if (m_Stat.IsOk())
  {
    if (DirExists(GetFullPath()))
    {
      return wxFileIconsTable::folder;
    }
    else if (!GetExt().empty())
    {
      return wxTheFileIconsTable->GetIconID(GetExt());
    }
    else
    {
      return wxFileIconsTable::file;
    }
  }
  else
  {
    return wxFileIconsTable::computer;
  }
}
