////////////////////////////////////////////////////////////////////////////////
// Name:      dir.cpp
// Purpose:   Implementation of class wex::dir
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/dir.h>
#include <wex/common/stream.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/reflection.h>
#include <wx/translation.h>

#include <thread>

namespace fs = std::filesystem;

namespace wex
{
class string_dir : public dir
{
public:
  string_dir(
    const std::string&        path,
    const wex::data::dir&     data,
    std::vector<std::string>& v)
    : dir(wex::path(path), data)
    , m_container(v)
  {
    ;
  }

private:
  bool on_dir(const path& p) const final
  {
    m_container.emplace_back(
      data().type().test(data::dir::RECURSIVE) ? p.string() : p.filename());
    return true;
  };

  bool on_file(const path& p) const final
  {
    m_container.emplace_back(
      data().type().test(data::dir::RECURSIVE) ? p.string() : p.filename());
    return true;
  };

  std::vector<std::string>& m_container;
};

class path_dir : public dir
{
public:
  path_dir(const path& p, const wex::data::dir& data, std::vector<path>& v)
    : dir(p, data)
    , m_container(v)
  {
    ;
  }

private:
  bool on_dir(const path& p) const final
  {
    m_container.emplace_back(p);
    return true;
  }

  bool on_file(const path& p) const final
  {
    m_container.emplace_back(p);
    return true;
  }

  std::vector<path>& m_container;
};

bool allow_hidden(const std::filesystem::path& p, const data::dir& data)
{
  for (auto it = p.begin(); it != p.end(); ++it)
  {
    if (
      it->string() != ".." && it->string() != "." &&
      it->string().starts_with("."))
    {
      // This file is hidden, only allow if flag HIDDEN is set.
      return data.type().test(data::dir::HIDDEN);
    }
  }

  return true;
}
}; // namespace wex

wex::dir::dir(
  const wex::path&      dir,
  const wex::data::dir& data,
  wxEvtHandler*         eh)
  : m_dir(dir)
  , m_data(data)
  , m_eh(eh)
  , m_tool(ID_TOOL_ADD)
{
}

int wex::dir::find_files()
{
  if (!m_dir.dir_exists())
  {
    log("traverse invalid path")
      << m_dir.string() << "current" << path::current().string();
    return 0;
  }

  if (!start())
  {
    log::trace("traverse busy") << m_dir;
    log::status(_("Busy"));
    return 0;
  }

  m_statistics.clear();

  if (m_data.vcs() != nullptr)
  {
    m_data.vcs()->setup_exclude(m_dir);
  }

  if (m_eh != nullptr)
  {
    std::thread t(
      [*this]
      {
        run();
      });

    t.detach();

    return 1;
  }

  return run();
}

bool wex::dir::find_files(const tool& tool)
{
  if (m_eh == nullptr)
  {
    return false;
  }

  m_tool = tool;

  return find_files() > 0;
}

void wex::dir::find_files_end() const
{
  log::status(m_tool.info(&m_statistics.get_elements()));

  if (m_eh != nullptr)
  {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_LIST_MATCH_FINISH);
    wxPostEvent(m_eh, event);
  }
}

int wex::dir::matches() const
{
  return m_statistics.get_elements().get(_("Files"));
}

bool wex::dir::on_dir(const path& p) const
{
  if (m_eh != nullptr)
  {
    if (!m_tool.is_find_type() && m_data.type().test(data::dir::DIRS))
    {
      m_statistics.get_elements().inc(_("Folders"));
      post_event(p);
    }
  }

  return true;
}

bool wex::dir::on_file(const path& p) const
{
  if (m_eh != nullptr)
  {
    if (m_tool.is_find_type())
    {
      stream s(m_data.find_replace_data(), p, m_tool, m_eh);

      if (!s.run_tool())
      {
        end();
        return false;
      }

      m_statistics += s.get_statistics();
    }
    else
    {
      post_event(p);
    }
  }

  return true;
}

void wex::dir::post_event(const path& p) const
{
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_LIST_MATCH);
  event.SetClientData(new wex::path_match(p));
  wxPostEvent(m_eh, event);
}

int wex::dir::run() const
{
  reflection reflect(
    {REFLECT_ADD("path", m_dir.string()),
     REFLECT_ADD("on dirs", m_data.dir_spec()),
     REFLECT_ADD("on files", m_data.file_spec()),
     REFLECT_ADD("flags", m_data.type().to_string())},
    reflection::log_t::SKIP_EMPTY);

  const auto id(std::hash<std::thread::id>{}(std::this_thread::get_id()));

  log::trace("thread") << id << "started" << reflect;

  try
  {
    if (m_data.type().test(data::dir::RECURSIVE))
    {
      for (auto i = fs::recursive_directory_iterator(m_dir.data());
           i != fs::recursive_directory_iterator();
           ++i)
      {
        if (
          fs::is_directory(i->path()) &&
          ((i->path().filename().string().starts_with(".") &&
            !m_data.type().test(data::dir::HIDDEN)) ||
           (m_data.vcs() != nullptr &&
            m_data.vcs()->is_dir_excluded(i->path()))))
        {
          i.disable_recursion_pending();
        }
        else
        {
          if (!traverse(*i))
          {
            log::trace("iterating aborted");
            return matches();
          }
        }
      }
    }
    else
    {
      if (fs::directory_iterator di(m_dir.data()), end; !std::all_of(
            di,
            end,
            [&](const fs::directory_entry& p)
            {
              return traverse(p);
            }))
      {
        log::trace("iterating aborted");
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

  log::trace("thread") << id << "ended matches:" << matches() << reflect.log();

  end();

  find_files_end();

  return matches();
}

bool wex::dir::traverse(const fs::directory_entry& e) const
{
  if (fs::is_regular_file(e.path()))
  {
    if (
      (m_data.vcs() == nullptr ||
       (m_data.vcs() != nullptr &&
        !m_data.vcs()->is_file_excluded(e.path()))) &&
      m_data.type().test(data::dir::FILES) && allow_hidden(e.path(), m_data) &&
      matches_one_of(e.path().filename().string(), m_data.file_spec()))
    {
      if (on_file(e.path()))
      {
        dir::get_statistics().inc_actions();
      }
    }
  }
  else if (
    m_data.dir_spec().empty() ||
    matches_one_of(e.path().filename().string(), m_data.dir_spec()))
  {
    on_dir(e.path());

    if (!m_data.dir_spec().empty())
    {
      dir::get_statistics().inc_actions();
    }
  }

  if (m_data.max_matches() != -1 && matches() >= m_data.max_matches())
  {
    log::trace("traverse limit reached") << m_data.max_matches();
    return false;
  }

  return interruptible::is_running();
}

std::vector<std::string>
wex::get_all_files(const std::string& path, const wex::data::dir& data)
{
  std::vector<std::string> v;
  string_dir               dir(path, data, v);
  dir.find_files();
  return v;
}

std::vector<wex::path>
wex::get_all_files(const path& p, const wex::data::dir& data)
{
  std::vector<path> v;
  path_dir          dir(p, data, v);
  dir.find_files();
  return v;
}
