////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wx/mimetype.h>
#include <wx/translation.h>
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
}; // namespace wex

wex::path::path(const fs::path& p, log_t t)
  : m_path(p)
  , m_stat(p.string())
  , m_log(t)
{
  if (p.empty())
  {
    m_path_original = current().data();
  }
}

wex::path::path(const path& p, const std::string& name, log_t t)
  : path(fs::path(substitute_tilde(p.string())).append(name).string(), t)
{
}

wex::path::path(const std::string& p, log_t t)
  : path(fs::path(substitute_tilde(p)), t)
{
}

wex::path::path(const char* p, log_t t)
  : path(fs::path(p), t)
{
}

wex::path::path(const path& r, log_t t)
  : path(r.data(), t)
{
}

wex::path::path(const std::vector<std::string>& v, log_t t)
  : path(std::filesystem::path(), t)
{
  for (const auto& it : v)
  {
    append(path(it));
  }
}

wex::path::~path()
{
  if (!m_path_original.empty() && m_path_original != current().data())
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
    m_stat          = r.m_stat;
  }

  return *this;
}

wex::path& wex::path::append(const wex::path& path)
{
  m_path /= fs::path(path.data());

  return *this;
}

void wex::path::current(const wex::path& p)
{
  if (!p.empty())
  {
    try
    {
      fs::current_path(p.data());
      log::trace("path current") << p;
    }
    catch (const std::exception& e)
    {
      wex::log(e) << "path:" << p;
    }
  }
}

wex::path wex::path::current()
{
  try
  {
    return path(fs::current_path());
  }
  catch (const std::exception& e)
  {
    wex::log(e) << "current";
    return path();
  }
}

bool wex::path::dir_exists() const
{
  return fs::is_directory(m_path);
}

bool wex::path::file_exists() const
{
  return filename().size() < 255 && fs::is_regular_file(m_path);
}

std::stringstream wex::path::log() const
{
  std::stringstream ss;

  if (!m_log.test(LOG_PATH))
  {
    ss << filename();
  }
  else
  {
    ss << m_path.string();
  }

  if (m_log.test(LOG_SYNC) || m_log.test(LOG_MOD))
  {
    if (stat().is_ok())
    {
      const std::string what =
        (m_log.test(LOG_SYNC) ? _("Synchronized") : _("Modified"));

      ss << " " << what << " " << stat().get_modification_time();
    }
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
  std::copy(m_path.begin(), m_path.end(), back_inserter(v));
  return v;
}

wex::path& wex::path::replace_filename(const std::string& filename)
{
  m_path.replace_filename(filename);

  return *this;
}
