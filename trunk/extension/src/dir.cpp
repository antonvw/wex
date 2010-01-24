/******************************************************************************\
* File:          dir.cpp
* Purpose:       Implementation of class 'wxExDir' and 'wxExDirOpenFile'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/dir.h>
#include <wx/extension/frame.h>
#include <wx/extension/util.h>

class wxExDirTraverser: public wxDirTraverser
{
public:
  wxExDirTraverser(wxExDir& dir)
    : m_Dir(dir)
    , m_Window(wxExGetYieldWindow()) {}

  virtual wxDirTraverseResult OnDir(const wxString& dirname)
  {
    if (m_Dir.Cancelled())
    {
      return wxDIR_STOP;
    }

    if (m_Dir.GetFlags() & wxDIR_DIRS)
    {
      m_Dir.OnDir(dirname);
    }

    if (m_Window != NULL)
    {
      wxTheApp->SafeYield(m_Window, true);
    }

    return wxDIR_CONTINUE;
  }

  virtual wxDirTraverseResult OnFile(const wxString& filename)
  {
    if (m_Dir.Cancelled())
    {
      return wxDIR_STOP;
    }

    if (wxExMatchesOneOf(filename, m_Dir.GetFileSpec()))
    {
      m_Dir.OnFile(filename);
    }

    if (m_Window != NULL)
    {
      wxTheApp->SafeYield(m_Window, true);
    }

    return wxDIR_CONTINUE;
  }

private:
  wxExDir& m_Dir;
  wxWindow* m_Window;
};

bool wxExDir::m_Cancelled = false;
bool wxExDir::m_IsBusy = false;

wxExDir::wxExDir(const wxString& fullpath, const wxString& filespec, int flags)
  : wxDir(fullpath)
  , m_FileSpec(filespec)
  , m_Flags(flags)
{
}

size_t wxExDir::FindFiles()
{
  if (!IsOpened()) return 0;

  if (m_IsBusy)
  {
    return 0;
  }

  m_Cancelled = false;
  m_IsBusy = true;

  // Using m_FileSpec here does not work, as it might
  // contain several specs (*.cpp;*.h), wxDir does not handle that.
  // Do not combine into one, Ubuntu complains.
  wxExDirTraverser traverser(*this);
  const size_t retValue = Traverse(traverser, wxEmptyString, m_Flags);

  m_IsBusy = false;

  return retValue;
}

wxExDirOpenFile::wxExDirOpenFile(wxExFrame* frame,
  const wxString& fullpath, 
  const wxString& filespec, 
  long file_flags,
  int dir_flags)
  : wxExDir(fullpath, filespec, dir_flags)
  , m_Frame(frame)
  , m_Flags(file_flags)
{
}

void wxExDirOpenFile::OnFile(const wxString& file)
{
  m_Frame->OpenFile(file, 0, wxEmptyString, m_Flags);
}
