////////////////////////////////////////////////////////////////////////////////
// Name:      stat.cpp
// Purpose:   Implementation of wex::file_stat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#include <io.h>
#endif
#include <iomanip>
#include <sstream>
#include <wex/stat.h>
#include <wx/filefn.h>
#ifdef __UNIX__
#include <unistd.h>
#endif

const std::string
wex::file_stat::get_creation_time(const std::string& format) const
{
  std::tm*          tm = std::localtime(&st_ctime);
  std::stringstream ss;
  ss << std::put_time(tm, format.c_str());
  return ss.str();
}

const std::string
wex::file_stat::get_modification_time(const std::string& format) const
{
  std::tm*          tm = std::localtime(&st_mtime);
  std::stringstream ss;
  ss << std::put_time(tm, format.c_str());
  return ss.str();
}

bool wex::file_stat::is_readonly() const
{
#ifdef _MSC_VER
  return (m_is_ok && ((st_mode & wxS_IWUSR) == 0));
#else
  return (m_is_ok && access(m_fullpath.c_str(), W_OK) == -1);
#endif
}

bool wex::file_stat::sync()
{
  if (m_fullpath.empty())
  {
    m_is_ok = false;
  }
  else
  {
#ifdef _MSC_VER
    m_is_ok = (stat(m_fullpath.c_str(), this) != -1);
#else
    m_is_ok = (::stat(m_fullpath.c_str(), this) != -1);
#endif
  }

  return m_is_ok;
}
