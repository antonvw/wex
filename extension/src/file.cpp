////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wxExFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/file.h>

wxExFile::wxExFile(bool open_file)
  : m_OpenFile(open_file)
  , m_HasRead(false)
  , m_FileName()
  , m_Stat()
{
}

wxExFile::wxExFile(
  const wxFileName& filename,
  wxFile::OpenMode mode,
  bool open_file)
  : wxFile(filename.GetFullPath(), mode)
  , m_OpenFile(open_file)
  , m_HasRead(false)
  , m_Stat(filename.GetFullPath())
{
  m_FileName.Assign(filename);
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

  if (
    m_FileName.m_Stat.Sync() &&
    m_FileName.m_Stat.st_mtime != m_Stat.st_mtime)
  {
    // Do not check return value,
    // we sync anyhow, to force nex time no sync.
    Get(true);

    // Update the stat member, so next time no sync.
    m_Stat.Sync();
      
    return true;
  }
  
  return false;
}

bool wxExFile::FileLoad(const wxExFileName& filename)
{
  Assign(filename);
  return m_FileName.FileExists() && MakeAbsolute() && Get(false);
}

void wxExFile::FileNew(const wxExFileName& filename)
{
  // Do not check return value, as file will not (yet) 
  // exist.
  Assign(filename);
  DoFileNew();
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

  if (m_OpenFile && !Open(m_FileName.GetFullPath(), wxFile::write))
  {
    return false;
  }

  DoFileSave(save_as);

  if (IsOpened())
  {
    Close();
  }

  ResetContentsChanged();
  
  m_FileName.m_Stat.Sync();
  m_Stat.Sync();

  return true;
}

bool wxExFile::Get(bool synced)
{
  if ( 
     synced && !Open(m_FileName.GetFullPath()) ||
    !synced && m_OpenFile && !Open(m_FileName.GetFullPath()))
  {
    return false;
  }

  if (!DoFileLoad(synced))
  {
    if (IsOpened())
    {
      Close();
    }
    
    return false;
  }

  if (IsOpened())
  {
    Close();
  }

  ResetContentsChanged();

  return true;
}

const wxCharBuffer wxExFile::Read(wxFileOffset seek_position)
{
  wxASSERT(IsOpened());
  
  if ((!m_HasRead && seek_position > 0) || 
      ( m_HasRead && Tell() != seek_position))
  {
    Seek(seek_position);
  }

  const wxFileOffset bytes_to_read = Length() - seek_position;

  wxCharBuffer buffer(bytes_to_read);

  if (wxFile::Read(buffer.data(), bytes_to_read) != bytes_to_read)
  {
    wxFAIL;
  }
  
  m_HasRead = true;

  return buffer;
}
