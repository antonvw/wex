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
#include <wx/extension/app.h> // for wxExApp
#include <wx/extension/base.h> // for wxExFrame

wxExFile::wxExFile()
  : m_FileName()
  , m_Stat()
  , m_Message(_("Select File"))
  , m_Wildcard(wxFileSelectorDefaultWildcardStr)
{
}

wxExFile::wxExFile(const wxString& filename, wxFile::OpenMode mode)
  : wxFile(filename, mode)
  , m_FileName(filename)
  , m_Stat(filename)
  , m_Message(_("Select File"))
  , m_Wildcard(wxFileSelectorDefaultWildcardStr)
{
  MakeAbsolute();
}

int wxExFile::AskFileOpen(wxFileDialog& dlg, bool ask_for_continue)
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

bool wxExFile::FileSave()
{
  wxFile::Close();

  return MakeAbsolute();
}

bool wxExFile::FileSaveAs()
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

void wxExFile::FileSync()
{
  if (FileOpen(m_FileName))
  {
    m_FileName.StatusText(STAT_SYNC | STAT_FULLPATH);
  }
}

bool wxExFile::MakeAbsolute()
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

wxCharBuffer wxExFile::Read(wxFileOffset seek_position)
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
      return wxFileIconsTable::computer;
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
  // Of course, if the lexers are not yet constructed, skip the rest.
  if (wxExApp::GetLexers() == NULL) return;

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

#if wxUSE_STATUSBAR
void wxExFileName::StatusText(long flags) const
{
  wxString text; // clear status bar for empty or not existing or not initialized file names

  if (IsOk())
  {
    const wxString path = (flags & STAT_FULLPATH
      ? GetFullPath(): GetFullName());

    text += path;

    if (m_Stat.IsOk())
    {
      const wxString what = (flags & STAT_SYNC
        ? _("Synchronized"): _("Modified"));
      const wxString time = (flags & STAT_SYNC
        ? wxDateTime::Now().Format(): m_Stat.GetModificationTime());
      text += " " + what + " " + time;
    }
  }

  wxExFrame::StatusText(text);
}
#endif // wxUSE_STATUSBAR

const wxString wxExStat::GetModificationTime(const wxString& format) const
{
  return wxDateTime(st_mtime).Format(format);
}

bool wxExStat::IsLink() const
{
#ifdef __UNIX__
  return m_IsOk && (S_ISLNK(st_mode) != 0);
#else // S_ISLNK not known
  return false;
#endif
}

bool wxExStat::SetReadOnly(const bool read_only)
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
