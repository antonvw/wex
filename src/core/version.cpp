////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/version.h>
#include <wx/translation.h>

#include <iomanip>
#include <sstream>

const wex::version_info wex::get_version_info()
{
  return version_info(
    {"wex",
     22,
     10,
     0,
     _("wex library (a library that offers windows ex and vi components)"),
     "(c) 1998-2022, Anton van Wezenbeek." + _("All rights reserved.")});
}

wex::version_info::version_info(const wxVersionInfo& info)
  : m_version(info)
{
}

const std::string wex::version_info::copyright() const
{
  return m_version.GetCopyright();
}

const std::string wex::version_info::description() const
{
  return m_version.GetDescription();
}

const std::string wex::version_info::get() const
{
  std::stringstream ss;
  ss << m_version.GetMajor() << "." << std::setfill('0') << std::setw(2)
     << m_version.GetMinor() << "." << m_version.GetMicro();
  return ss.str();
}
