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
#include <unistd.h>
#include <wx/extension/stat.h>

const std::string wxExStat::GetModificationTime() const 
{
  std::tm* tm = std::localtime(&st_mtime);
  std::stringstream ss;
  ss << std::put_time(tm, "%c");
  return ss.str();
}

bool wxExStat::IsReadOnly() const 
{
#ifdef _MSC_VER
  return (m_IsOk && _access(m_FullPath.c_str(), 4) == -1);
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
