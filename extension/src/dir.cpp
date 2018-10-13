////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wex::dir and wex::open_file_dir
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

namespace wex
{
  /// Collects files into container.
  template <class T>
  class container_dir : public dir
  {
  public:
    container_dir(const T & path, const std::string& filespec, int flags) 
    : dir(path, filespec, flags) {;};

    const auto & Get() const {return m_Container;};
  private:
    virtual bool OnDir(const path& p) override {
      m_Container.emplace_back(p);
      return true;};
    virtual bool OnFile(const path& p) override {
      m_Container.emplace_back(p);
      return true;};

    std::vector <T> m_Container;
  };

  class string_dir : public dir
  {
  public:
    string_dir(const std::string & path, const std::string& filespec, int flags) 
    : dir(path, filespec, flags) {;};

    const auto & Get() const {return m_Container;};
  private:
    virtual bool OnDir(const path& p) override {
      m_Container.emplace_back(p.GetFullName());
      return true;};
    virtual bool OnFile(const path& p) override {
      m_Container.emplace_back(p.GetFullName());
      return true;};

    std::vector <std::string> m_Container;
  };
};

std::vector <wex::path> wex::get_all_files(
  const wex::path& path, const std::string& filespec, int flags) 
{
  wex::container_dir<wex::path> dir(path, filespec, flags);
  dir.FindFiles();
  return dir.Get();
}

std::vector <std::string> wex::get_all_files(
  const std::string& path, const std::string& filespec, int flags) 
{
  wex::string_dir dir(path, filespec, flags);
  dir.FindFiles();
  return dir.Get();
}

namespace fs = std::experimental::filesystem;

wex::dir::dir(const wex::path& dir, const std::string& filespec, int flags)
  : m_Dir(dir)
  , m_FileSpec(filespec)
  , m_Flags(flags)
{
}

bool Handle(const fs::directory_entry& e, wex::dir* dir, int& matches)
{
  if (fs::is_regular_file(e.path()))
  {
    if ((dir->GetFlags() & wex::DIR_FILES) && 
      wex::matches_one_of(e.path().filename().string(), dir->GetFileSpec()))
    {
      dir->OnFile(e.path());
      matches++;
    }
  }
  else if ((dir->GetFlags() & wex::DIR_DIRS) && fs::is_directory(e.path()) &&
    wex::matches_one_of(e.path().filename().string(), dir->GetFileSpec()))
  {
    dir->OnDir(e.path());
  }

  return !wex::interruptable::Cancelled();
}

int wex::dir::FindFiles()
{
  if (!m_Dir.DirExists())
  {
    wex::log("invalid path") << m_Dir.Path().string();
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
    if (m_Flags & wex::DIR_RECURSIVE)
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
    wex::log(e) << "filesystem";
  }

  Stop();

  return matches;
}

wex::open_file_dir::open_file_dir(wex::frame* frame,
  const wex::path& path, 
  const std::string& filespec, 
  wex::stc_window_flags file_flags,
  int dir_flags)
  : wex::dir(path, filespec, dir_flags)
  , m_Frame(frame)
  , m_Flags(file_flags)
{
}

bool wex::open_file_dir::OnFile(const wex::path& file)
{
  m_Frame->OpenFile(file, wex::stc_data().Flags(m_Flags));
  return true;
}
