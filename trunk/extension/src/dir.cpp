/******************************************************************************\
* File:          dir.cpp
* Purpose:       Implementation of class 'exDir'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/dir.h>
#include <wx/extension/util.h>

class exDirTraverser: public wxDirTraverser
{
public:
  exDirTraverser(exDir& dir, wxArrayString& files, bool callonfile) 
    : m_Dir(dir)
    , m_CallOnFile(callonfile)
    , m_Files(files)
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

      if (m_CallOnFile)
      {
        m_Dir.OnFile(dirname);
      }
    }

    if (wxIsMainThread())
    {
      wxTheApp->Yield();
    }
    else
    {
      wxThread::This()->Yield();
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
      
      if (m_CallOnFile)
      {
        m_Dir.OnFile(filename);
      }
    }

    if (wxIsMainThread() && wxTheApp != NULL)
    {
      wxTheApp->Yield();
    }
    else
    {
      wxThread::This()->Yield();
    }

    return wxDIR_CONTINUE;
  }

private:
  exDir& m_Dir;
  bool m_CallOnFile;
  wxArrayString& m_Files;
};

exDir::exDir(const wxString& fullpath, const wxString& filespec)
  : wxDir(fullpath)
  , m_FileSpec(filespec)
  , m_Flags(wxDIR_DEFAULT)
{
}

exDir::~exDir()
{
}

size_t exDir::FindFiles(int flags, bool callOnFile)
{
  if (!IsOpened()) return 0;

  m_Flags = flags;

  exDirTraverser traverser(*this, m_Files, callOnFile);

  // TODO: Using m_FileSpec here does not work, as it might
  // contains several specs (*.cpp;*.h), wxDir does not handle that.
  Traverse(traverser, wxEmptyString, m_Flags);
  
  return m_Files.Count();
}
