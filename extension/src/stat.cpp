////////////////////////////////////////////////////////////////////////////////
// Name:      stat.cpp
// Purpose:   Implementation of wxExStat class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
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
const std::string wxExStat::GetModificationTime() const 
{
#ifdef _MSC_VER
  return wxDateTime(st_mtime).Format("%c").ToStdString();
#else
  std::tm* tm = std::localtime(&st_mtime);
  std::stringstream ss;
  ss << std::put_time(tm, "%c");
  return ss.str();
#endif
}

bool wxExStat::IsReadOnly() const 
{
#ifdef _MSC_VER
  return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));
//  return (m_IsOk && _access(m_FullPath.c_str(), 4) == -1);
#else
  return (m_IsOk && access(m_FullPath.c_str(), W_OK) == -1);
#endif
}

bool wxExStat::Sync() 
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
