////////////////////////////////////////////////////////////////////////////////
// Name:      file-status.cpp
// Purpose:   Implementation of wex::file_status class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/chrono.h>
#include <wex/core/file-status.h>

#ifdef _MSC_VER
#include <io.h>
#endif
#include <wx/filefn.h>
#ifdef __UNIX__
#include <unistd.h>
#endif

namespace wex
{
std::ostream& operator<<(std::ostream& os, const wex::file_status& rhs)
{
  os << rhs.m_fullpath;

  return os;
}
} // namespace wex

const std::string
wex::file_status::get_creation_time_str(const std::string& format) const
{
  return chrono(format).get_time(m_file_status.st_ctime);
}

const std::string
wex::file_status::get_modification_time_str(const std::string& format) const
{
  return chrono(format).get_time(m_file_status.st_mtime);
}

time_t wex::file_status::get_access_time() const
{
  return m_file_status.st_atime;
}

time_t wex::file_status::get_creation_time() const
{
  return m_file_status.st_ctime;
}

time_t wex::file_status::get_modification_time() const
{
  return m_file_status.st_mtime;
}

off_t wex::file_status::get_size() const
{
  return m_file_status.st_size;
}

bool wex::file_status::is_readonly() const
{
  // using perms = fs::status(m_fullpath).permissions() and checking on
  // ((perms & fs::perms::owner_write) == fs::perms::none) &&
  // ((perms & fs::perms::group_write) == fs::perms::none) is not ok,
  // e.g -rw-r--r--  1 root  wheel  213 Jan  1  2020 /etc/hosts
  // has w for owner, but we should check access for current user
#ifdef _MSC_VER
  return (m_is_ok && ((m_file_status.st_mode & wxS_IWUSR) == 0));
#else
  const auto err = access(m_fullpath.c_str(), W_OK);

  return (m_is_ok && err == -1 && errno != ENOENT);
#endif
}

bool wex::file_status::sync()
{
  if (m_fullpath.empty())
  {
    m_is_ok = false;
  }
  else
  {
#ifdef _MSC_VER
    m_is_ok = (stat(m_fullpath.c_str(), &m_file_status) != -1);
#else
    m_is_ok = (::stat(m_fullpath.c_str(), &m_file_status) != -1);
#endif
  }

  return m_is_ok;
}
