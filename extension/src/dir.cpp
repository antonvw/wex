////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dir.h>
#include <wx/extension/frame.h>
#include <wx/extension/util.h>

class wxExDirTraverser: public wxDirTraverser
{
public:
  wxExDirTraverser(wxExDir& dir)
    : m_Dir(dir){;}
  
  // Calling Yield on Linux Mint causes the app to hang,
  // so enabled it only for MSW.

  virtual wxDirTraverseResult OnDir(const wxString& dirname) override
  {
    if (m_Dir.Cancelled())
    {
      return wxDIR_STOP;
    }

    if (m_Dir.GetFlags() & wxDIR_DIRS)
    {
      m_Dir.OnDir(dirname);
    }

    if (wxTheApp != NULL)
    {
#ifdef __WXMSW__      
      wxTheApp->Yield();
#endif      
    }

    return wxDIR_CONTINUE;
  }

  virtual wxDirTraverseResult OnFile(const wxString& filename) override
  {
    if (m_Dir.Cancelled())
    {
      return wxDIR_STOP;
    }

    if (wxExMatchesOneOf(filename, m_Dir.GetFileSpec()))
    {
      m_Dir.OnFile(filename);
    }

    if (wxTheApp != NULL)
    {
#ifdef __WXMSW__
      wxTheApp->Yield();
#endif      
    }

    return wxDIR_CONTINUE;
  }

private:
  wxExDir& m_Dir;
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
  if (!IsOpened() || m_IsBusy)
  {
    return 0;
  }

  m_Cancelled = false;
  m_IsBusy = true;

  wxExDirTraverser traverser(*this);
  
  // Using m_FileSpec only works if it 
  // contains single spec (*.h), wxDir does not handle multi specs (*.cpp; *.h).
  const size_t retValue = Traverse(
    traverser, 
    (m_FileSpec.Contains(";") ? wxString(wxEmptyString): m_FileSpec), 
    m_Flags);

  m_IsBusy = false;

  return retValue;
}

#if wxUSE_GUI
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
#endif
