////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wxExFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/extension/file.h>

wxExFile& wxExFile::operator=(const wxExFile& f)
{
  if (this != &f)
  {
    m_IsLoaded = f.m_IsLoaded;
    m_OpenFile = f.m_OpenFile;
    m_Path = f.m_Path;
    m_Stat = f.m_Stat;
    m_File = std::make_unique<wxFile>(m_Path.GetFullPath());
  }

  return *this;
}

bool wxExFile::CheckSync()
{
  // Might be used without wxApp.
  wxConfigBase* config = wxConfigBase::Get(false);

  if ( m_File->IsOpened() ||
      !m_Path.m_Stat.IsOk() ||
      (config != nullptr && !config->ReadBool("AllowSync", true)))
  {
    return false;
  }

  if (m_Path.m_Stat.Sync())
  {
    bool sync_needed = false;
    
    if (m_Path.m_Stat.st_mtime != m_Stat.st_mtime)
    {
      // Do not check return value,
      // we sync anyhow, to force nex time no sync.
      Get(true);
      
      sync_needed = true;
    }
    
    if (m_Path.m_Stat.IsReadOnly() != m_Stat.IsReadOnly())
    {
      sync_needed = true;
    }

    if (sync_needed)
    {
      // Update the stat member, so next time no sync.
      if (!m_Stat.Sync())
      {
        // This might be reported in an OnIdle, so do not use wxLogError.
        wxLogStatus("Could not sync: ", m_Path.GetFullPath().c_str());
      }
        
      return true;
    }
  }
  
  return false;
}

bool wxExFile::FileLoad(const wxExPath& file)
{
  Assign(file);
  return m_Path.FileExists() && MakeAbsolute() && Get(false);
}

void wxExFile::FileNew(const wxExPath& file)
{
  Assign(file);
  DoFileNew();
}

bool wxExFile::FileSave(const wxExPath& file)
{
  bool save_as = false;

  if (!file.GetFullPath().empty())
  {
    Assign(file);
    MakeAbsolute();
    save_as = true;
  }

  if (!save_as && !m_IsLoaded)
  {
    wxLogStatus("File has not been loaded");
    return false;
  }

  if (m_OpenFile && !Open(m_Path.GetFullPath(), wxFile::write))
  {
    return false;
  }

  DoFileSave(save_as);

  Close();

  ResetContentsChanged();
  
  m_Path.m_Stat.Sync();
  m_Stat.Sync();

  return true;
}

bool wxExFile::Get(bool synced)
{
  // Do not log error messages if opening fails.
  {
    wxLogNull logNo;
  
    if ( 
       (synced && !Open(m_Path.GetFullPath())) ||
      (!synced && m_OpenFile && !Open(m_Path.GetFullPath())))
    {
      return false;
    }
  }

  if (!DoFileLoad(synced))
  {
    Close();
    return false;
  }

  Close();
  
  m_IsLoaded = true;

  ResetContentsChanged();

  return true;
}

bool wxExFile::Open(const std::string& file, wxFile::OpenMode mode, int access)
{
  return m_File->Open(file, mode, access);
}

const wxCharBuffer* wxExFile::Read(wxFileOffset seek_position)
{
  wxASSERT(m_File->IsOpened());
  
  if ((m_Buffer.get() != nullptr && seek_position > 0) || 
      (m_Buffer.get() != nullptr && m_File->Tell() != seek_position))
  {
    m_File->Seek(seek_position);
  }

  m_Buffer = std::make_unique<wxCharBuffer>(m_File->Length() - seek_position);

  if (m_File->Read(m_Buffer->data(), m_Buffer->length()) != m_Buffer->length())
  {
    wxFAIL;
  }

  return m_Buffer.get();
}
