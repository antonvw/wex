/******************************************************************************\
* File:          dir.cpp
* Purpose:       Implementation of class 'exDir'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/extension.h>
#include <wx/extension/dir.h>

class exDirTraverser: public wxDirTraverser
{
public:
  exDirTraverser(exDir& dir, wxArrayString& files, wxStatusBar* statusbar) 
    : m_Dir(dir)
    , m_Files(files)
    , m_StatusBar(statusbar)
    {}

  virtual wxDirTraverseResult OnDir(const wxString& dirname)
  {
    if (m_Dir.Cancelled())
    {
      return wxDIR_STOP;
    }

    if (m_Dir.GetFlags() & wxDIR_DIRS)
    {
      m_Files.Add(dirname);
    }

    if (wxIsMainThread())
    {
      wxTheApp->Yield();
    }
    else
    {
      wxThread::This()->Yield();
    }

    if (m_StatusBar != NULL)
    {
      m_StatusBar->SetStatusText(dirname);
    }

    return wxDIR_CONTINUE;
  }

  virtual wxDirTraverseResult OnFile(const wxString& filename)
  {
    if (m_Dir.Cancelled())
    {
      return wxDIR_STOP;
    }

    wxFileName file(filename);

    if (exMatchesOneOf(file, m_Dir.GetFileSpec()))
    {
      m_Files.Add(filename);
    }

    if (wxIsMainThread())
    {
      wxTheApp->Yield();
    }
    else
    {
      wxThread::This()->Yield();
    }

    if (m_StatusBar != NULL)
    {
      m_StatusBar->SetStatusText(filename);
    }

    return wxDIR_CONTINUE;
  }

private:
  exDir& m_Dir;
  wxArrayString& m_Files;
  wxStatusBar* m_StatusBar;
};

exDir::exDir(const wxString& fullpath, const wxString& filespec, wxStatusBar* statusbar)
  : wxDir(fullpath)
  , m_FileSpec(filespec)
  , m_Flags(wxDIR_DEFAULT)
  , m_StatusBar(statusbar)
{
}

exDir::~exDir()
{
}

size_t exDir::FindFiles(int flags, bool callOnFile)
{
  if (!IsOpened()) return 0;

  m_Flags = flags;

  exFrame::StatusText(_("Collecting files"));

  exDirTraverser traverser(*this, m_Files, m_StatusBar);

  // TODO: Using m_FileSpec here does not work, as it might
  // contains several specs (*.cpp;*.h), wxDir does not handle that.
  Traverse(traverser, wxEmptyString, m_Flags); // m_FileSpec

  if (callOnFile)
  {
    int files_processed = 0;

    for (size_t i = 0; i < m_Files.Count() && !Cancelled(); i++)
    {
      OnFile(m_Files[i]);
      files_processed++;
    }

    return files_processed;
  }
  
  exStatusText(_("Ready"));

  return m_Files.Count();
}
