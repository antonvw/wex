/******************************************************************************\
* File:          file.cpp
* Purpose:       Implementation of wxWidgets file extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/stdpaths.h> // strangely enough, for wxTheFileIconsTable
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/extension/file.h>
#include <wx/extension/app.h> // for wxExApp

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
     !m_FileName.GetStat().IsOk() ||
     !wxExApp::GetConfigBool("AllowSync", true))
  {
    return false;
  }

  if (m_FileName.GetStat().Sync())
  {
    if (m_FileName.GetStat().st_mtime != m_Stat.st_mtime)
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

  // Do not make it absolute, the specified filename does not need
  // to exist.
}

bool wxExFile::FileLoad(const wxExFileName& filename)
{
  // First set the member, even if filename does not exist.
  m_FileName = filename;

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
  if (!filename.empty())
  {
    m_FileName.Assign(filename);
    m_FileName.SetLexer();
  }

  if (!Open(m_FileName.GetFullPath(), wxFile::write))
  {
    return false;
  }

  DoFileSave();
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
      m_FileName.GetStat().Sync(m_FileName.GetFullPath()) &&
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

void wxExFileName::SetLexer(
  const wxString& lexer,
  const wxString& text)
{
  if (wxExApp::GetLexers() == NULL) 
  {
    m_Lexer = wxExLexer();
  }
  else
  {
    if (lexer.empty())
    {
      if (text != "forced")
      {
        m_Lexer = wxExApp::GetLexers()->FindByFileName(*this);

        if (m_Lexer.GetScintillaLexer().empty() && !text.empty())
        {
          m_Lexer = wxExApp::GetLexers()->FindByText(text);
        }
      }
      else
      {
        m_Lexer = wxExLexer();
      }
    }
    else
    {
      m_Lexer = wxExApp::GetLexers()->FindByName(lexer);
    }
  }
}

const wxString wxExStat::GetModificationTime() const
{
  return wxDateTime(st_mtime).FormatISOCombined(' ');
}

bool wxExStat::Sync() 
{
#ifdef __WXGTK__
  m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
#else
  m_IsOk = (stat(m_FullPath.c_str(), this) != -1);
#endif
  return m_IsOk;
}

bool wxExStat::Sync(const wxString& fullpath) 
{
  m_FullPath = fullpath;
  return Sync();
}
