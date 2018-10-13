////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <iomanip>	
#include <sstream>
#include <wx/versioninfo.h>
#include <wx/extension/version.h>

const wex::version_info wex::get_version_info() 
{
  return version_info("wex", 
    19, 4, 0, 
    "wex Library (a collection of wxWidgets extension classes)",
    "(c) 1998-2018, Anton van Wezenbeek." + std::string(_("All rights reserved.")));
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

const std::string wex::version_info::Copyright() const
{
  return m_version.GetCopyright();
}

const std::string wex::version_info::Description() const
{
  return m_version.GetDescription();
}

const std::string wex::version_info::Get() const
{
  std::stringstream ss;
  ss << 
    m_version.GetMajor() << "." << std::setfill('0') << std::setw(2) << 
    m_version.GetMinor() << "." << 
    m_version.GetMicro();
  return ss.str();
}
