////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wex::dir and wex::open_file_dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <wex/dir.h>
#include <wex/frame.h>
#include <wex/log.h>
#include <wex/util.h>

namespace fs = std::filesystem;

namespace wex
{
  /// Collects files into container.
  template <class T>
  class container_dir : public dir
  {
  public:
    container_dir(
      const T & path, 
      const std::string& filespec, 
      const std::string& dirspec, 
      dir::type_t flags) 
    : dir(path, filespec, dirspec, flags) {;};

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
      const std::string & path, 
      const std::string& filespec, 
      const std::string& dirspec, 
      dir::type_t flags) 
    : dir(path, filespec, dirspec, flags) {;};

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

  bool traverse(const fs::directory_entry& e, wex::dir* dir, int& matches)
  {
    if (fs::is_regular_file(e.path()))
    {
      if (
        dir->type().test(dir::FILES) && 
        matches_one_of(e.path().filename().string(), dir->file_spec()))
      {
        dir->on_file(e.path());
        matches++;
      }
    }
    else if (
       dir->type().test(dir::DIRS) && fs::is_directory(e.path()) &&
      (dir->dir_spec().empty() || 
       matches_one_of(e.path().filename().string(), dir->dir_spec())))
    {
      dir->on_dir(e.path());
      
      if (!dir->dir_spec().empty())
      {
        matches++;
      }
    }

    return !interruptable::is_cancelled();
  }
};

std::vector <wex::path> wex::get_all_files(
  const wex::path& path, 
  const std::string& filespec, 
  const std::string& dirspec, 
  dir::type_t flags) 
{
  container_dir<wex::path> dir(path, filespec, dirspec, flags);
  dir.find_files();
  return dir.get();
}

std::vector <std::string> wex::get_all_files(
  const std::string& path, 
  const std::string& filespec, 
  const std::string& dirspec, 
  dir::type_t flags) 
{
  string_dir dir(path, filespec, dirspec, flags);
  dir.find_files();
  return dir.get();
}

wex::dir::dir(
  const wex::path& dir, 
  const std::string& filespec, 
  const std::string& dirspec, 
  type_t flags)
  : m_Dir(dir)
  , m_DirSpec(dirspec)
  , m_FileSpec(filespec)
  , m_Flags(flags)
{
}

int wex::dir::find_files()
{
  if (!m_Dir.dir_exists())
  {
    log("invalid path") << m_Dir;
    return -1;
  }

  if (!start())
  {
    log::status(_("Busy"));
    return -1;
  }

  int matches = 0;

  try
  {
    if (m_Flags.test(RECURSIVE))
    {
      fs::recursive_directory_iterator rdi(m_Dir.data(),
#ifdef __WXMSW__
        fs::directory_options::none), end;
#else
        fs::directory_options::skip_permission_denied), end;
#endif
      if (!std::all_of(rdi, end, [&] (const fs::directory_entry& p) 
        {return traverse(p, this, matches);}))
      {
        log("recursive_directory_iterator") << m_Dir;
      }
    }
    else
    {
      fs::directory_iterator di(m_Dir.data()), end;
      if (!std::all_of(di, end, [&] (const fs::directory_entry& p) 
        {return traverse(p, this, matches);}))
      {
        log("directory_iterator") << m_Dir;
      }
    }
  }
  catch (fs::filesystem_error& e)
  {
    log(e) << "filesystem";
  }

  stop();

  log::verbose("iterating") << m_Dir 
    << "on files:" << m_FileSpec 
    << "on dirs:" << m_DirSpec 
    << "flags:" << m_Flags 
    << "matches:" << matches;

  return matches;
}

wex::open_file_dir::open_file_dir(wex::frame* frame,
  const wex::path& path, 
  const std::string& filespec, 
  stc_data::window_t file_flags,
  dir::type_t type)
  : dir(path, filespec, std::string(), type)
  , m_Frame(frame)
  , m_Flags(file_flags)
{
}

bool wex::open_file_dir::on_file(const wex::path& file)
{
  m_Frame->open_file(file, stc_data().flags(m_Flags));
  return true;
}
