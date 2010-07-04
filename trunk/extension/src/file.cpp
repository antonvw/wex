////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class 'wxExFile'
// Author:    Anton van Wezenbeek
// Created:   2010-03-18
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/file.h>

wxExFile::wxExFile()
  : m_FileName()
  , m_Stat()
{
}

wxExFile::wxExFile(const wxExFileName& filename, wxFile::OpenMode mode)
    : wxFile(filename.GetFullPath(), mode)
{
  Assign(filename);
  MakeAbsolute();
}

bool wxExFile::CheckSync()
{
  // Might be used without wxApp.
  wxConfigBase* config = wxConfigBase::Get(false);

  if ( IsOpened() ||
      !m_FileName.m_Stat.IsOk() ||
      (config != NULL && !config->ReadBool("AllowSync", true)))
  {
    return false;
  }

  if (m_FileName.m_Stat.Sync())
  {
    if (m_FileName.m_Stat.st_mtime != m_Stat.st_mtime)
    {
      Get(true);

      // Update the stat member, so next time no sync.
      m_Stat.Sync();
      
      return true;
    }
  }
  
  return false;
}

bool wxExFile::FileLoad(const wxExFileName& filename)
{
  Assign(filename);
  return m_FileName.FileExists() && MakeAbsolute() && Get(false);
}

bool wxExFile::FileNew(const wxExFileName& filename)
{
  if (!Assign(filename))
  {
    return false;
  }

  DoFileNew();

  return true;
}

bool wxExFile::FileSave(const wxExFileName& filename)
{
  bool save_as = false;

  if (filename.IsOk())
  {
    Assign(filename);
    MakeAbsolute();
    save_as = true;
  }

  if (!Open(m_FileName.GetFullPath(), wxFile::write))
  {
    return false;
  }

  DoFileSave(save_as);
  Close();
  ResetContentsChanged();
  
  m_FileName.m_Stat.Sync();
  m_Stat.Sync();

  return true;
}

bool wxExFile::Get(bool synced)
{
  if (Open(m_FileName.GetFullPath()))
  {
    DoFileLoad(synced);
    Close();
    ResetContentsChanged();
    return true;
  }
  
  return false;
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
