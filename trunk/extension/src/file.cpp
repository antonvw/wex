////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class 'wxExFile'
// Author:    Anton van Wezenbeek
// Created:   2010-03-18
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>
#include <wx/extension/file.h>

wxExFile::wxExFile()
  : m_FileName()
  , m_Stat()
{
}

wxExFile::wxExFile(const wxString& filename, wxFile::OpenMode mode)
  : wxFile(filename, mode)
{
  Assign(filename);
  MakeAbsolute();
}

void wxExFile::Assign(const wxFileName& filename)
{
  m_FileName = filename;
  m_Stat = filename.GetFullPath();
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

bool wxExFile::FileLoad(const wxString& filename)
{
  Assign(filename);

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

void wxExFile::FileNew(const wxString& filename)
{
  Assign(filename);

  DoFileNew();
}

bool wxExFile::FileSave(const wxString& filename)
{
  bool save_as = false;

  if (!filename.empty())
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

  return true;
}

void wxExFile::FileSync()
{
  if (Open(m_FileName.GetFullPath()))
  {
    DoFileLoad(true);

    Close();
    ResetContentsChanged();
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
