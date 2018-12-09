////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wex::dir and wex::open_file_dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <wex/dir.h>
#include <wex/frame.h>
#include <wex/log.h>
#include <wex/util.h>
#include <easylogging++.h>

namespace wex
{
  /// Collects files into container.
  template <class T>
  class container_dir : public dir
  {
  public:
    container_dir(
      const T & path, const std::string& filespec, dir::type_t flags) 
    : dir(path, filespec, flags) {;};

    const auto & get() const {return m_Container;};
  private:
    virtual bool on_dir(const path& p) override {
      m_Container.emplace_back(p);
      return true;};
    virtual bool on_file(const path& p) override {
      m_Container.emplace_back(p);
      return true;};

    std::vector <T> m_Container;
  };

  class string_dir : public dir
  {
  public:
    string_dir(
      const std::string & path, const std::string& filespec, dir::type_t flags) 
    : dir(path, filespec, flags) {;};

    const auto & get() const {return m_Container;};
  private:
    virtual bool on_dir(const path& p) override {
      m_Container.emplace_back(p.fullname());
      return true;};
    virtual bool on_file(const path& p) override {
      m_Container.emplace_back(p.fullname());
      return true;};

    std::vector <std::string> m_Container;
  };
};

std::vector <wex::path> wex::get_all_files(
  const wex::path& path, const std::string& filespec, dir::type_t flags) 
{
  wex::container_dir<wex::path> dir(path, filespec, flags);
  dir.find_files();
  return dir.get();
}

std::vector <std::string> wex::get_all_files(
  const std::string& path, const std::string& filespec, dir::type_t flags) 
{
  wex::string_dir dir(path, filespec, flags);
  dir.find_files();
  return dir.get();
}

namespace fs = std::filesystem;

wex::dir::dir(const wex::path& dir, const std::string& filespec, type_t flags)
  : m_Dir(dir)
  , m_FileSpec(filespec)
  , m_Flags(flags)
{
}

bool traverse(const fs::directory_entry& e, wex::dir* dir, int& matches)
{
  if (fs::is_regular_file(e.path()))
  {
    if (dir->type().test(wex::dir::FILES) && 
      wex::matches_one_of(e.path().filename().string(), dir->file_spec()))
    {
      dir->on_file(e.path());
      matches++;
    }
  }
  else if (dir->type().test(wex::dir::DIRS) && fs::is_directory(e.path()))
  {
    dir->on_dir(e.path());
  }

  return !wex::interruptable::is_cancelled();
}

int wex::dir::find_files()
{
  if (!m_Dir.dir_exists())
  {
    wex::log("invalid path") << m_Dir.data().string();
    return -1;
  }

  if (!start())
  {
    wex::log_status(_("Busy"));
    return -1;
  }

  VLOG(9) << "iterating: " << m_Dir.data() << " on: " << m_FileSpec << " flags: " << m_Flags;

  int matches = 0;

  try
  {
    if (m_Flags.test(RECURSIVE))
    {
      const fs::directory_options options = 
#ifdef __WXMSW__
        fs::directory_options::none;
#else
        fs::directory_options::skip_permission_denied;
#endif
      
      for (const auto& p: fs::recursive_directory_iterator(m_Dir.data(), options))
      {
        if (!traverse(p, this, matches)) break;
      }
    }
    else
    {
      for (const auto& p: fs::directory_iterator(m_Dir.data()))
      {
        if (!traverse(p, this, matches)) break;
      }
    }
  }
  catch (fs::filesystem_error& e)
  {
    wex::log(e) << "filesystem";
  }

  stop();

  return matches;
}

wex::open_file_dir::open_file_dir(wex::frame* frame,
  const wex::path& path, 
  const std::string& filespec, 
  wex::stc_data::window_t file_flags,
  dir::type_t type)
  : wex::dir(path, filespec, type)
  , m_Frame(frame)
  , m_Flags(file_flags)
{
}

bool wex::open_file_dir::on_file(const wex::path& file)
{
  m_Frame->open_file(file, wex::stc_data().flags(m_Flags));
  return true;
}
