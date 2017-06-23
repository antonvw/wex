////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <experimental/filesystem>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dir.h>
#include <wx/extension/frame.h>
#include <wx/extension/path.h>
#include <wx/extension/util.h>

namespace fs = std::experimental::filesystem;

wxExDir::wxExDir(const std::string& dir, const std::string& filespec, int flags)
  : m_Dir(dir)
  , m_FileSpec(filespec)
  , m_Flags(flags)
{
}

bool wxExDir::DirExists() const 
{
  return fs::is_directory(m_Dir);
}

bool Handle(const fs::directory_entry& e, wxExDir* dir, int& matches)
{
  if (fs::is_regular_file(e.path()))
  {
    if (wxExMatchesOneOf(e.path().filename().string(), dir->GetFileSpec()))
    {
      dir->OnFile(e.path().string());
      matches++;
    }
  }
  else if (fs::is_directory(e.path()))
  {
    dir->OnDir(e.path().string());
  }

  return !wxExInterruptable::Cancelled();
}

int wxExDir::FindFiles()
{
  if (!DirExists())
  {
    std::cout << "Could not open: " << m_Dir << "\n";
    return -1;
  }
  else if (Running())
  {
    wxLogStatus(_("Busy"));
    return -1;
  }

  int matches = 0;

  Start();

  if (m_Flags & DIR_DIRS)
  {
    for (auto& p: fs::recursive_directory_iterator(m_Dir))
    {
      if (!Handle(p, this, matches)) break;
    }
  }
  else
  {
    for (auto& p: fs::directory_iterator(m_Dir))
    {
      if (!Handle(p, this, matches)) break;
    }
  }

  Stop();

  return matches;
}

#if wxUSE_GUI
wxExDirOpenFile::wxExDirOpenFile(wxExFrame* frame,
  const std::string& fullpath, 
  const std::string& filespec, 
  wxExSTCWindowFlags file_flags,
  int dir_flags)
  : wxExDir(fullpath, filespec, dir_flags)
  , m_Frame(frame)
  , m_Flags(file_flags)
{
}

bool wxExDirOpenFile::OnFile(const std::string& file)
{
  m_Frame->OpenFile(file, wxExSTCData().Flags(m_Flags));
  return true;
}
#endif
