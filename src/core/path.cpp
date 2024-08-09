////////////////////////////////////////////////////////////////////////////////
// Name:      path.cpp
// Purpose:   Implementation of class wex::path
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/url.hpp>

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wx/mimetype.h>
#include <wx/translation.h>
#include <wx/utils.h>

namespace fs = std::filesystem;

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
  : path(fs::path(p.string()).append(name), t)
{
}

wex::path::path(const std::string& p, log_t t)
  : path(
      fs::path(boost::algorithm::replace_all_copy(p, "~", wxGetHomeDir())),
      t)
{
}

wex::path::path(const char* p, log_t t)
  : path(fs::path(p), t)
{
}

wex::path::path(const path& r)
  : path(r.data(), r.m_log)
{
}

wex::path::path(const std::vector<std::string>& v, log_t t)
  : path(fs::path(), t)
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
      log::trace("change dir") << p.string();
      fs::current_path(p.data());
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

bool wex::path::exists() const
{
  return fs::exists(m_path);
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

      ss << " " << what << " " << stat().get_modification_time_str();
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
    if (boost::urls::url_view view(m_path.string()); view.has_scheme())
    {
      return browser(m_path.string());
    }

    return false;
  }

  const auto* type(
    wxTheMimeTypesManager->GetFileTypeFromExtension(extension()));

  if (type == nullptr)
  {
    return false;
  }

  const std::string command(type->GetOpenCommand(string()));

  if (command.empty())
  {
    return false;
  }

  // wex:: process, boost::process::system, std::system all do not work
  // so use wx
  if (wxExecute(command) == -1)
  {
    wex::log("open_mime execute") << command;
    return false;
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
