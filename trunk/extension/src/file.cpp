/******************************************************************************\
* File:          file.cpp
* Purpose:       Implementation of wxExtension file classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h> // strangely enough, for wxTheFileIconsTable
#include <wx/stdpaths.h> // strangely enough, for wxTheFileIconsTable
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/extension/file.h>
#include <wx/extension/lexers.h>

wxExFile::wxExFile()
  : m_FileName()
  , m_Stat()
{
}

wxExFile::wxExFile(const wxString& filename, wxFile::OpenMode mode)
  : wxFile(filename, mode)
  , m_FileName(filename)
  , m_Stat(filename)
{
  MakeAbsolute();
}

bool wxExFile::CheckFileSync()
{
  if (IsOpened() ||
     !m_FileName.m_Stat.IsOk() ||
     !wxConfigBase::Get()->ReadBool("AllowSync", true))
  {
    return false;
  }

  if (m_FileName.m_Stat.Sync())
  {
    if (m_FileName.m_Stat.st_mtime != m_Stat.st_mtime)
    {
      FileSync();

      // Update the stat member, so next time no sync.
      m_Stat.Sync();

      // Now we synced, so always return true.
      return true;
    }
  }

  return false;
}

void wxExFile::FileNew(const wxExFileName& filename)
{
  m_FileName = filename;
  m_Stat = filename.GetFullPath();

  // Do not make it absolute, the specified filename does not need
  // to exist.
}

bool wxExFile::FileLoad(const wxExFileName& filename)
{
  // First set the member, even if filename does not exist.
  m_FileName = filename;
  m_Stat = filename.GetFullPath();

  if (!m_FileName.FileExists()) return false;

  if (MakeAbsolute())
  {
    if (Open(m_FileName.GetFullPath()))
    {
      DoFileLoad();
      Close();
      ResetContentsChanged();
      return true;
    }
  }

  return false;
}

bool wxExFile::FileSave(const wxString filename)
{
  if (!GetContentsChanged())
  {
    return false;
  }

  bool save_as = false;

  if (!filename.empty())
  {
    m_FileName = filename;
    save_as = true;
  }

  if (!Open(m_FileName.GetFullPath(), wxFile::write))
  {
    return false;
  }

  DoFileSave(save_as);
  MakeAbsolute();
  Close();
  ResetContentsChanged();

  return true;
}

void wxExFile::FileSync()
{
  if (Open(m_FileName.GetFullPath()))
  {
    DoFileLoad(true);
    Close();
  }
}

bool wxExFile::MakeAbsolute()
{
  if (m_FileName.MakeAbsolute())
  {
    return 
      m_FileName.m_Stat.Sync(m_FileName.GetFullPath()) &&
      m_Stat.Sync(m_FileName.GetFullPath());
  }
  else
  {
    return false;
  }
}

const wxCharBuffer wxExFile::Read(wxFileOffset seek_position)
{
  const wxFileOffset bytes_to_read = Length() - seek_position;

  // Always do a seek, so you can do more Reads on the same object.
  Seek(seek_position);

  wxCharBuffer buffer(bytes_to_read);

  if (wxFile::Read(buffer.data(), bytes_to_read) != bytes_to_read)
  {
    wxFAIL;
  }

  return buffer;
}

wxExFileName::wxExFileName(const wxString& fullpath, wxPathFormat format)
  : wxFileName(fullpath, format)
  , m_Stat(fullpath) 
{
  m_Lexer = wxExLexers::Get()->FindByFileName(*this);
}

wxExFileName::wxExFileName(const wxFileName& filename)
  : wxFileName(filename)
  , m_Stat(filename.GetFullPath()) 
{
  m_Lexer = wxExLexers::Get()->FindByFileName(*this);
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
