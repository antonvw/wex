////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include <wex/core/version.h>
#include <wx/translation.h>

const wex::version_info wex::get_version_info()
{
  return version_info(
    {"wex",
     26,
     4,
     0,
     0,
     _("wex library (a library that offers windows ex and vi components)"),
     "(c) 1998-2025, Anton van Wezenbeek. " + _("All rights reserved.")});
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

const std::string wex::version_info::get(exclude_t type) const
{
  if (type.none())
  {
    return m_version.GetVersionString();
  }

  std::stringstream version;

  if (!type.test(EXCLUDE_NAME))
  {
    version << m_version.GetName() << " ";
  }

  version << m_version.GetMajor() << "." << m_version.GetMinor();

  if (
    !type.test(EXCLUDE_MICRO) &&
    (m_version.GetMicro() != 0 || m_version.GetRevision() != 0))
  {
    version << "." << m_version.GetMicro();
  }

  if (!type.test(EXCLUDE_MICRO) && m_version.GetRevision() != 0)
  {
    version << "." << m_version.GetRevision();
  }

  return version.str();
}
