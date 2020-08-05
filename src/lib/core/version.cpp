////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/version.hpp>
#ifndef __WXMSW__
#include <easylogging++.h>
#endif
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <iomanip>
#include <sstream>
#include <wex/version.h>
#include <wx/versioninfo.h>

const wex::version_info wex::get_version_info()
{
  return version_info(
    {"wex",
     20,
     10,
     0,
     "wex Library (a library that offers windows ex and vi components)",
     "(c) 1998-2020, Anton van Wezenbeek." +
       std::string(_("All rights reserved."))});
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

const std::stringstream wex::version_info::external_libraries() const
{
  auto json(nlohmann::json::meta());

  std::stringstream ss;
  ss << wex::get_version_info().description() << "\n"
     << "Boost Libraries version: " << BOOST_VERSION / 100000
     << "."                               // major version
     << BOOST_VERSION / 100 % 1000 << "." // minor version
     << BOOST_VERSION % 100               // patch level
     << "\n"
     << json.meta()["name"] << ": " << json.meta()["version"]["string"] << "\n"
     << "pugixml version: " << PUGIXML_VERSION / 1000 << "." // major version
     << PUGIXML_VERSION % 1000 / 10 << "."                   // minor version
     << PUGIXML_VERSION % 10                                 // patch level
     << "\n"
#ifndef __WXMSW__
     << "easylogging++ version: " << el::VersionInfo::version() << "\n"
#endif
     << wxGetLibraryVersionInfo().GetDescription().c_str();

  return ss;
}

const std::string wex::version_info::get() const
{
  std::stringstream ss;
  ss << m_version.GetMajor() << "." << std::setfill('0') << std::setw(2)
     << m_version.GetMinor() << "." << m_version.GetMicro();
  return ss.str();
}
