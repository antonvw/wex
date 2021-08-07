////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wex::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>
#include <wex/core.h>
#include <wex/dir.h>
#include <wex/log.h>

namespace fs = std::filesystem;

namespace wex
{
  class string_dir : public dir
  {
  public:
    string_dir(const std::string& path, const wex::data::dir& data)
      : dir(path, data)
    {
      ;
    };

    const auto& get() const { return m_container; };

  private:
    bool on_dir(const path& p) override
    {
      m_container.emplace_back(
        data().type().test(data::dir::RECURSIVE) ? p.string(): p.fullname());
      return true;
    };
    bool on_file(const path& p) override
    {
      m_container.emplace_back(
        data().type().test(data::dir::RECURSIVE) ? p.string(): p.fullname());
      return true;
    };

    std::vector<std::string> m_container;
  };

  bool traverse(const fs::directory_entry& e, wex::dir* dir, int& matches)
  {
    if (fs::is_regular_file(e.path()))
    {
      if (
        dir->data().type().test(data::dir::FILES) &&
        matches_one_of(e.path().filename().string(), dir->data().file_spec()))
      {
        dir->on_file(e.path());
        matches++;
      }
    }
    else if (
      dir->data().type().test(data::dir::DIRS) && fs::is_directory(e.path()) &&
      (dir->data().dir_spec().empty() ||
       matches_one_of(e.path().filename().string(), dir->data().dir_spec())))
    {
      dir->on_dir(e.path());

      if (!dir->data().dir_spec().empty())
      {
        matches++;
      }
    }

    if (dir->data().max_matches() != -1 && matches >= dir->data().max_matches())
    {
      return false;
    }

    return !interruptible::is_cancelled();
  }
}; // namespace wex

std::vector<std::string>
wex::get_all_files(const std::string& path, const wex::data::dir& data)
{
  string_dir dir(path, data);
  dir.find_files();
  return dir.get();
}

wex::dir::dir(const wex::path& dir, const wex::data::dir& data)
  : m_dir(dir)
  , m_data(data)
{
}

int wex::dir::find_files()
{
  if (!m_dir.dir_exists())
  {
    log("invalid path") << m_dir;
    return -1;
  }

  if (!start())
  {
    log::status(_("Busy"));
    return -1;
  }

  int  matches = 0;
  bool all_off = false;

  try
  {
    if (m_data.type().test(data::dir::RECURSIVE))
    {
      fs::recursive_directory_iterator rdi(
        m_dir.data(),
#ifdef __WXMSW__
        fs::directory_options::none),
        end;
#else
        fs::directory_options::skip_permission_denied),
        end;
#endif
      if (std::all_of(rdi, end, [&](const fs::directory_entry& p) {
            return traverse(p, this, matches);
          }))
      {
        all_off = true;
      }
    }
    else
    {
      fs::directory_iterator di(m_dir.data()), end;
      if (std::all_of(di, end, [&](const fs::directory_entry& p) {
            return traverse(p, this, matches);
          }))
      {
        all_off = true;
      }
    }
  }
  catch (fs::filesystem_error& e)
  {
    log(e) << "filesystem";
  }
  catch (std::exception& e)
  {
    log(e) << "exception";
  }

  stop();

  log::trace("iterating")
    << m_dir << "on files:" << m_data.file_spec()
    << "on dirs:" << m_data.dir_spec() << "flags:" << m_data.type()
    << "matches:" << matches << "all_off:" << all_off;

  return matches;
}
