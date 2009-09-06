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
#include <wx/extension/base.h> // for wxExFrame

wxExFile::wxExFile()
  : m_FileName()
  , m_Stat()
  , m_Wildcard(wxFileSelectorDefaultWildcardStr)
{
}

wxExFile::wxExFile(const wxString& filename, wxFile::OpenMode mode)
  : wxFile(filename, mode)
  , m_FileName(filename)
  , m_Stat(filename)
  , m_Wildcard(wxFileSelectorDefaultWildcardStr)
{
  MakeAbsolute();
}

int wxExFile::ShowFileDialog(wxFileDialog& dlg, bool ask_for_continue)
{
  if (ask_for_continue)
  {
    if (!Continue())
    {
      return wxID_CANCEL;
    }
  }

  // Take care that if current filename exists, we set
  // the directory member and filename, so that is where the dialog will open.
  if (m_FileName.FileExists())
  {
    dlg.SetFilename(m_FileName.GetFullPath());
    dlg.SetDirectory(m_FileName.GetPath());
  }

  m_Wildcard = dlg.GetWildcard();

  return dlg.ShowModal();
}

bool wxExFile::CheckSyncNeeded()
{
  if (IsOpened() ||
     !m_FileName.GetStat().IsOk() ||
     !wxExApp::GetConfigBool("AllowSync", true))
  {
    return false;
  }

  if (m_FileName.GetStat().st_mtime != m_Stat.st_mtime)
  {
    FileSync();
  }

  return true;
}

bool wxExFile::Continue()
{
  if (GetContentsChanged())
  {
    if (!m_FileName.FileExists())
    {
      switch (wxMessageBox(
        _("Save changes") + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES:    if (!FileSaveAs()) return false; break;
        case wxNO:     ResetContentsChanged(); break;
        case wxCANCEL: return false; break;
      }
    }
    else
    {
      switch (wxMessageBox(
        _("Save changes to") + ": " + m_FileName.GetFullPath() + "?",
        _("Confirm"),
        wxYES_NO | wxCANCEL | wxICON_QUESTION))
      {
        case wxYES:    FileSave(); break;
        case wxNO:     ResetContentsChanged(); break;
        case wxCANCEL: return false; break;
      }
    }
  }

  return true;
}

bool wxExFile::FileNew(const wxExFileName& filename)
{
  if (!Continue()) return false;

  m_FileName = filename;

  // Do not make it absolute, the specified filename does not need
  // to exist.
  return true;
}

bool wxExFile::FileOpen(const wxExFileName& filename)
{
  if (!Continue()) return false;

  // First set the member, even if filename does not exist.
  m_FileName = filename;

  if (!m_FileName.FileExists()) return false;

  if (MakeAbsolute())
  {
    return Open(m_FileName.GetFullPath());
  }
  else
  {
    return false;
  }
}

bool wxExFile::FileSave()
{
  wxFile::Close();

  return MakeAbsolute();
}

bool wxExFile::FileSaveAs()
{
  wxASSERT(wxTheApp != NULL);

  wxFileDialog dlg(
    wxTheApp->GetTopWindow(),
    wxFileSelectorPromptStr,
    wxEmptyString,
    wxEmptyString,
    m_Wildcard,
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (ShowFileDialog(dlg, false) == wxID_CANCEL)
  {
    return false;
  }

  const wxString filename = dlg.GetPath();

  if (!filename.empty())
  {
    m_FileName.Assign(filename);
    m_FileName.SetLexer();
    return FileSave();
  }

  return false;
}

void wxExFile::FileSync()
{
  if (FileOpen(m_FileName))
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(m_FileName, STAT_SYNC | STAT_FULLPATH);
#endif
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

int wxExFileName::GetIcon() const
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

const wxString wxExStat::GetModificationTime(const wxString& format) const
{
  return wxDateTime(st_mtime).Format(format);
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
