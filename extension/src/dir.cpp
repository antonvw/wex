////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <experimental/filesystem>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/dir.h>
#include <wx/extension/frame.h>
#include <wx/extension/log.h>
#include <wx/extension/util.h>
#include <easylogging++.h>

/// Collects files into container.
template <class T>
class wxExDirToContainer : public wxExDir
{
public:
  wxExDirToContainer(const T & path, const std::string& filespec, int flags) 
  : wxExDir(path, filespec, flags) {;};

  const auto & Get() const {return m_Container;};
private:
  virtual bool OnDir(const wxExPath& p) override {
    m_Container.emplace_back(p);
    return true;};
  virtual bool OnFile(const wxExPath& p) override {
    m_Container.emplace_back(p);
    return true;};

  std::vector <T> m_Container;
};

class wxExDirToString : public wxExDir
{
public:
  wxExDirToString(const std::string & path, const std::string& filespec, int flags) 
  : wxExDir(path, filespec, flags) {;};

  const auto & Get() const {return m_Container;};
private:
  virtual bool OnDir(const wxExPath& p) override {
    m_Container.emplace_back(p.GetFullName());
    return true;};
  virtual bool OnFile(const wxExPath& p) override {
    m_Container.emplace_back(p.GetFullName());
    return true;};

  std::vector <std::string> m_Container;
};

std::vector <wxExPath> wxExGetAllFiles(
  const wxExPath& path, const std::string& filespec, int flags) 
{
  wxExDirToContainer<wxExPath> dir(path, filespec, flags);
  dir.FindFiles();
  return dir.Get();
}

std::vector <std::string> wxExGetAllFiles(
  const std::string& path, const std::string& filespec, int flags) 
{
  wxExDirToString dir(path, filespec, flags);
  dir.FindFiles();
  return dir.Get();
}

namespace fs = std::experimental::filesystem;

wxExDir::wxExDir(const wxExPath& dir, const std::string& filespec, int flags)
  : m_Dir(dir)
  , m_FileSpec(filespec)
  , m_Flags(flags)
{
}

bool Handle(const fs::directory_entry& e, wxExDir* dir, int& matches)
{
  if (fs::is_regular_file(e.path()))
  {
    if ((dir->GetFlags() & DIR_FILES) && 
      wxExMatchesOneOf(e.path().filename().string(), dir->GetFileSpec()))
    {
      dir->OnFile(e.path());
      matches++;
    }
  }
  else if ((dir->GetFlags() & DIR_DIRS) && fs::is_directory(e.path()) &&
    wxExMatchesOneOf(e.path().filename().string(), dir->GetFileSpec()))
  {
    dir->OnDir(e.path());
  }

  return !wxExInterruptable::Cancelled();
}

int wxExDir::FindFiles()
{
  if (!m_Dir.DirExists())
  {
    wxExLog("invalid path") << m_Dir.Path().string();
    return -1;
  }

  if (!Start())
  {
    wxLogStatus(_("Busy"));
    return -1;
  }

  VLOG(9) << "iterating: " << m_Dir.Path() << " on: " << m_FileSpec << " flags: " << m_Flags;

  int matches = 0;

  try
  {
    if (m_Flags & DIR_RECURSIVE)
    {
      const fs::directory_options options = 
#ifdef __WXMSW__
        fs::directory_options::none;
#else
        fs::directory_options::skip_permission_denied;
#endif
      
      for (const auto& p: fs::recursive_directory_iterator(m_Dir.Path(), options))
      {
        if (!Handle(p, this, matches)) break;
      }
    }
    else
    {
      for (const auto& p: fs::directory_iterator(m_Dir.Path()))
      {
        if (!Handle(p, this, matches)) break;
      }
    }
  }
  catch (fs::filesystem_error& e)
  {
    wxExLog(e) << "filesystem";
  }

  Stop();

  return matches;
}

#if wxUSE_GUI
wxExDirOpenFile::wxExDirOpenFile(wxExFrame* frame,
  const wxExPath& path, 
  const std::string& filespec, 
  wxExSTCWindowFlags file_flags,
  int dir_flags)
  : wxExDir(path, filespec, dir_flags)
  , m_Frame(frame)
  , m_Flags(file_flags)
{
}

bool wxExDirOpenFile::OnFile(const wxExPath& file)
{
  m_Frame->OpenFile(file, wxExSTCData().Flags(m_Flags));
  return true;
}
#endif
