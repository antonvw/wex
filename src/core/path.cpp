////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wx/mimetype.h>
#include <wx/url.h>

namespace fs = std::filesystem;

namespace wex
{
  const std::string substitute_tilde(const std::string& text)
  {
    auto out(text);
    boost::algorithm::replace_all(out, "~", wxGetHomeDir());
    return out;
  }

  const std::string lexer_string(const std::string& filename)
  {
    lexers* l = lexers::get(false);
    return l != nullptr && !l->get_lexers().empty() ?
             l->find_by_filename(filename).display_lexer() :
             std::string();
  }
}; // namespace wex

wex::path::path(const fs::path& p, status_t t)
  : m_path(p)
  , m_stat(p.string())
  , m_lexer(lexer_string(p.filename().string()))
  , m_status(t)
{
  if (p.empty())
  {
    m_path_original = current();
  }
}

wex::path::path(const std::string& p, const std::string& name)
  : path(fs::path(substitute_tilde(p)).append(name).string())
{
}

wex::path::path(const std::string& p)
  : path(fs::path(substitute_tilde(p)))
{
}

wex::path::path(const char* p)
  : path(fs::path(p))
{
}

wex::path::path(const path& r, status_t t)
  : path(r.data(), t)
{
}

wex::path::path(const std::vector<std::string>& v)
  : path()
{
  for (const auto& it : v)
  {
    append(it);
  }
}

wex::path::~path()
{
  if (!m_path_original.empty() && m_path_original != current())
  {
    current(m_path_original);
  }
}

wex::path& wex::path::operator=(const wex::path& r)
{
  if (this != &r)
  {
    m_path          = r.data();
    m_path_original = r.m_path_original;
    m_lexer         = r.m_lexer;
    m_stat          = r.m_stat;
    m_status        = r.m_status;
  }

  return *this;
}

wex::path& wex::path::append(const wex::path& path)
{
  m_path /= fs::path(path.data());

  return *this;
}

void wex::path::current(const std::string& path)
{
  if (!path.empty())
  {
    try
    {
      fs::current_path(path);
      log::trace("path current") << path;
    }
    catch (const std::exception& e)
    {
      wex::log(e) << "path:" << path;
    }
  }
}

std::string wex::path::current()
{
  try
  {
    return fs::current_path().string();
  }
  catch (const std::exception& e)
  {
    wex::log(e) << "current";
    return std::string();
  }
}

bool wex::path::dir_exists() const
{
  return fs::is_directory(m_path);
}

bool wex::path::file_exists() const
{
  return fullname().size() < 255 && fs::is_regular_file(m_path);
}

std::stringstream wex::path::log() const
{
  std::stringstream ss;

  ss << (m_status[STAT_FULLPATH] || fullname().empty() ? string() : fullname());

  if (stat().is_ok())
  {
    const std::string what =
      (m_status[STAT_SYNC] ? _("Synchronized") : _("Modified"));

    ss << " " << what << " " << stat().get_modification_time();
  }

  return ss;
}

wex::path& wex::path::make_absolute()
{
  m_path = fs::absolute(m_path);
  m_stat.sync();

  if (!fs::is_directory(m_path.parent_path()))
  {
    m_path.clear();
  }

  return *this;
}

bool wex::path::open_mime() const
{
  if (extension().empty())
  {
    if (wxURL(m_path.string()).IsOk() || m_path.string().substr(0, 4) == "http")
    {
      return browser(m_path.string());
    }
    else
    {
      return false;
    }
  }
  else if (auto* type(
             wxTheMimeTypesManager->GetFileTypeFromExtension(extension()));
           type == nullptr)
  {
    return false;
  }
  else if (const std::string command(type->GetOpenCommand(string()));
           command.empty())
  {
    return false;
  }
  else
  {
    // wex:: process, boost::process::system, std::system all do not work
    // so use wx
    if (wxExecute(command) == -1)
    {
      wex::log("open_mime execute") << command;
      return false;
    }
  }

  return true;
}

const std::vector<wex::path> wex::path::paths() const
{
  std::vector<path> v;

  for (const auto& e : m_path)
  {
    v.emplace_back(e);
  }

  return v;
}

wex::path& wex::path::replace_filename(const std::string& filename)
{
  m_path.replace_filename(filename);

  return *this;
}
