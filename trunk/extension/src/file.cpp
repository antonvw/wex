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

#ifdef __WXMSW__
#include <io.h> // for chmod
#endif
#include <wx/stdpaths.h> // strangely enough, for wxTheFileIconsTable
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/extension/file.h>
#include <wx/extension/app.h> // for exApp
#include <wx/extension/base.h> // for exFrame

exFile::exFile()
  : m_FileName()
  , m_Stat()
  , m_Message(_("Select File"))
  , m_Wildcard(wxFileSelectorDefaultWildcardStr)
{
}

exFile::exFile(const wxString& filename, wxFile::OpenMode mode)
  : wxFile(filename, mode)
  , m_FileName(filename)
  , m_Stat(filename)
  , m_Message(_("Select File"))
  , m_Wildcard(wxFileSelectorDefaultWildcardStr)
{
  MakeAbsolute();
}

int exFile::AskFileOpen(wxFileDialog& dlg, bool ask_for_continue)
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

  m_Message = dlg.GetMessage();
  m_Wildcard = dlg.GetWildcard();

  return dlg.ShowModal();
}

bool exFile::CheckSyncNeeded()
{
  if (IsOpened() || 
     !m_FileName.GetStat().IsOk() || 
     !exApp::GetConfigBool("AllowSync", true))
  {
    return false;
  }

  if (m_FileName.GetStat().st_mtime != GetStat().st_mtime)
  {
    FileSync();
  }
  
  return true;
}

bool exFile::Continue()
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

bool exFile::FileNew(const exFileName& filename)
{
  if (!Continue()) return false;

  m_FileName = filename;
  
  // Do not make it absolute, the specified filename does not need
  // to exist.
  return true;
}

bool exFile::FileOpen(const exFileName& filename)
{
  if (
    !Continue() ||
    !filename.FileExists()) return false;

  m_FileName = filename;
  
  if (MakeAbsolute())
  {
    return Open(m_FileName.GetFullPath());
  }
  else
  {
    return false;
  }
}

bool exFile::FileSave()
{
  wxFile::Close();

  return MakeAbsolute();
}

bool exFile::FileSaveAs()
{
  wxFileDialog dlg(
    wxTheApp->GetTopWindow(),
    m_Message,
    wxEmptyString,
    wxEmptyString,
    m_Wildcard,
    wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

  if (AskFileOpen(dlg, false) == wxID_CANCEL)
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

void exFile::FileSync()
{
  if (FileOpen(m_FileName))
  {
    m_FileName.StatusText(STAT_SYNC | STAT_FULLPATH);
  }
}

bool exFile::MakeAbsolute()
{
  if (m_FileName.MakeAbsolute())
  {
    m_FileName.GetStat().Update(m_FileName.GetFullPath());
    m_Stat.Update(m_FileName.GetFullPath());
    return true;
  }
  else
  {
    return false;
  }
}

wxString* exFile::Read(wxFileOffset seek_position)
{
  const wxFileOffset bytes_to_read = Length() - seek_position;

  // Always do a seek, so you can do more Reads on the same object.
  Seek(seek_position);

  wxMemoryBuffer buffer(bytes_to_read);
  buffer.SetDataLen(bytes_to_read);

  if (wxFile::Read(buffer.GetData(), bytes_to_read) == bytes_to_read)
  {
    return new wxString(buffer, bytes_to_read);
  }
  else
  {
    wxLogError("Read error");
    return NULL;
  }
}

int exFileName::GetIcon() const
{
  if (GetStat().IsOk())
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
      return wxFileIconsTable::computer;
    }
  }
  else
  {
    return wxFileIconsTable::computer;
  }
}

void exFileName::SetLexer(
  const wxString& lexer,
  const wxString& text)
{
  // Of course, if the lexers are not yet constructed, skip the rest.
  if (exApp::GetLexers() == NULL) return;

  if (lexer.empty())
  {
    m_Lexer = exApp::GetLexers()->FindByFileName(*this);

    if (m_Lexer.GetScintillaLexer().empty() && !text.empty())
    {
      m_Lexer = exApp::GetLexers()->FindByText(text);
    }
  }
  else
  {
    m_Lexer = exApp::GetLexers()->FindByName(lexer);
  }
}

#if wxUSE_STATUSBAR
void exFileName::StatusText(long flags) const
{
  wxString text; // clear status bar for empty or not existing or not initialized file names

  if (IsOk())
  {
    const wxString path = (flags & STAT_FULLPATH
      ? GetFullPath(): GetFullName());

    text += path;

    if (GetStat().IsOk())
    {
      const wxString what = (flags & STAT_SYNC
        ? _("Synchronized"): _("Modified"));
      const wxString time = (flags & STAT_SYNC
        ? wxDateTime::Now().Format(): GetStat().GetModificationTime());
      text += " " + what + " " + time;
    }
  }

  exFrame::StatusText(text);
}
#endif // wxUSE_STATUSBAR

const wxString exStat::GetModificationTime(const wxString& format) const
{
  return wxDateTime(st_mtime).Format(format);
}

bool exStat::IsLink() const
{
#ifdef __UNIX__
  return m_IsOk && (S_ISLNK(st_mode) != 0);
#else // S_ISLNK not known
  return false;
#endif
}

bool exStat::SetReadOnly(const bool read_only)
{
  if (IsOk())
  {
    if (chmod(m_FullPath.c_str(),
          read_only ?
            st_mode & ~wxS_IWUSR:
            st_mode | wxS_IWUSR) == 0)
    {
      Update(m_FullPath);
      return IsOk();
    }
  }

  return false;
}
