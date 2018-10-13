////////////////////////////////////////////////////////////////////////////////
// Name:      stat.cpp
// Purpose:   Implementation of wex::stat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#include <io.h>
#endif
#include <iomanip>
#include <sstream>
#include <wx/datetime.h>
#include <wx/filefn.h>
#include <wx/extension/stat.h>
#ifdef __UNIX__
#include <unistd.h>
#endif

// See also GetTime in listview.cpp
const std::string wex::stat::GetModificationTime(
  const std::string& format) const 
{
#ifdef _MSC_VER
  return wxDateTime(st_mtime).Format(format).ToStdString();
#else
  std::tm* tm = std::localtime(&st_mtime);
  std::stringstream ss;
  ss << std::put_time(tm, format.c_str());
  return ss.str();
#endif
}

bool wex::stat::IsReadOnly() const 
{
#ifdef _MSC_VER
  return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));
#else
  return (m_IsOk && access(m_FullPath.c_str(), W_OK) == -1);
#endif
}

bool wex::stat::Sync() 
{
  if (m_FullPath.empty())
  {
    m_IsOk = false;
  }
  else
  {
#ifdef _MSC_VER
    m_IsOk = (stat(m_FullPath.c_str(), this) != -1);
#else
    m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
#endif
  }

  return m_IsOk;
}
