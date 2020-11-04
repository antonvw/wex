////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/version.hpp>
#include <iomanip>
#include <regex>
#include <sstream>
#ifndef __WXMSW__
#include <easylogging++.h>
#endif
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
#include <wex/version.h>
#include <wx/utils.h>
#include <wx/versioninfo.h>

#include <ctags/main/ctags.h>

namespace wex
{
  const std::string skip_quotes(const std::string& text)
  {
    return std::regex_replace(
      text,
      std::regex("\"+"),
      "",
      std::regex_constants::format_sed);
  }
} // namespace wex

const wex::version_info wex::get_version_info()
{
  return version_info(
    {"wex",
     21,
     4,
     0,
     "wex library (a library that offers windows ex and vi components)",
     "(c) 1998-2020, Anton van Wezenbeek." +
       std::string("All rights reserved.")});
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

  ss << wex::get_version_info().description() << ": " << get() << "\n"

     << "Boost library: " << BOOST_VERSION / 100000 << "." // major version
     << BOOST_VERSION / 100 % 1000 << "."                  // minor version
     << BOOST_VERSION % 100                                // patch level
     << "\n"

     << skip_quotes(json.meta()["name"])
     << " library: " << skip_quotes(json.meta()["version"]["string"]) << "\n"

     << "pugixml library: " << PUGIXML_VERSION / 1000 << "." // major version
     << PUGIXML_VERSION % 1000 / 10 << "."                   // minor version
     << PUGIXML_VERSION % 10                                 // patch level
     << "\n"

     // ctags
     << PROGRAM_NAME << ": " << PROGRAM_VERSION << "\n"

#ifndef __WXMSW__
     << "easylogging++ library: " << el::VersionInfo::version() << "\n"
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
