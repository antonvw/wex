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
#include <wx/extension/app.h> // for exApp
#include <wx/extension/util.h> // for exColourToLong

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
    /// \todo If the last char is a NULL, is not put in the string strangely enough.
    ///       However, it is in the buffer as next message shows.
//wxLogMessage("%d", ((char*)buffer.GetData())[bytes_to_read - 1]);
    return new wxString(buffer, *wxConvCurrent, bytes_to_read);
  }
  else
  {
    wxLogError(FILE_INFO("Read error"));
    return NULL;
  }
}


int exFileName::GetIcon() const
{
  if (GetStat().IsOk() && !GetExt().empty())
  {
    // README: DirExists from wxFileName is not okay, so use the static one here!
    return
      (wxFileName::DirExists(GetFullPath()) ?
       wxFileIconsTable::folder:
       wxTheFileIconsTable->GetIconID(GetExt()));
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

#if wxUSE_GUI
exConfigDialog* exStat::m_ConfigDialog = NULL;
#endif

#if wxUSE_GUI
// This is a static method, cannot use normal members here.
int exStat::ConfigDialog(
  const wxString& title,
  wxWindow* parent,
  wxWindowID id)
{
  std::vector<exConfigItem> items;
  items.push_back(exConfigItem(_("day"), CONFIG_COLOUR));
  items.push_back(exConfigItem(_("week"), CONFIG_COLOUR));
  items.push_back(exConfigItem(_("month"), CONFIG_COLOUR));
  items.push_back(exConfigItem(_("year"), CONFIG_COLOUR));
#ifndef __WXMSW__
  // Links only used on Unix.
  items.push_back(exConfigItem(_("link"), CONFIG_COLOUR));
#endif

  if (m_ConfigDialog == NULL)
  {
    m_ConfigDialog = new exConfigDialog(
      parent,
      exApp::GetConfig(),
      items,
      title,
      "Colour/",
      0, 2,
      wxOK | wxCANCEL | wxAPPLY,
      id);
  }

  return m_ConfigDialog->Show();
}
#endif

const wxColour exStat::GetColour() const
{
  if (IsOk())
  {
    const wxString& group = "Colour/";
    const wxDateTime now = wxDateTime::Now();
    const wxDateTime mtime = wxDateTime(st_mtime);

    // within 1 day (12 hours)
    if (mtime > now - 12 * wxTimeSpan::Hour())
    {
      return exApp::GetConfig(group + _("day"), exColourToLong(*wxGREEN));
    }
    // within 1 week
    else if (mtime > now - wxDateSpan::Week())
    {
      return exApp::GetConfig(group + _("week"), exColourToLong(*wxBLUE));
    }
    // within 1 month (default colour is white, so not used)
    else if (mtime > now - wxDateSpan::Month())
    {
      return exApp::GetConfig(group + _("month"), exColourToLong(*wxWHITE));
    }
    // within 1 year (default colour is white, so not used)
    else if (mtime > now - wxDateSpan::Year())
    {
      return exApp::GetConfig(group + _("year"), exColourToLong(*wxWHITE));
    }
  }

  return *wxWHITE;
}

const wxColour exStat::GetLinkColour() const
{
  return exApp::GetConfig("Colour/" + _("link"), exColourToLong(*wxRED));
}

const wxString exStat::GetModificationTime(const wxString& format) const
{
#ifdef __WXMSW__
  return wxDateTime(st_mtime).Format(format);
#else
  // TODO: The current locale %c cannot be sorted in the listview,
  // so override format and use this one.
  return wxDateTime(st_mtime).Format("%Y%m%d %H:%M:%S");
#endif
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
