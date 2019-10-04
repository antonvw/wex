////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <iomanip>	
#include <sstream>
#include <wx/versioninfo.h>
#include <wex/version.h>

const wex::version_info wex::get_version_info() 
{
  return version_info("wex", 
    20, 4, 0, 
    "wex Library (a library that offers windows ex and vi components)",
    "(c) 1998-2020, Anton van Wezenbeek." + std::string(_("All rights reserved.")));
}

wex::version_info::version_info(const std::string& name,
  int major,
  int minor,
  int micro,
  const std::string& description,
  const std::string& copyright)
  : m_version(name, major, minor, micro, description, copyright)
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
  ss << 
    m_version.GetMajor() << "." << std::setfill('0') << std::setw(2) << 
    m_version.GetMinor() << "." << 
    m_version.GetMicro();
  return ss.str();
}
