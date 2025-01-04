////////////////////////////////////////////////////////////////////////////////
// Name:      version.cpp
// Purpose:   Implementation of wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/version.h>
#include <wx/translation.h>

#include <sstream>

const wex::version_info wex::get_version_info()
{
  return version_info(
    {"wex",
     25,
     4,
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

const std::string wex::version_info::get(bool include_name) const
{
  return include_name ?
           m_version.GetVersionString().ToStdString() :
           find_after(m_version.GetVersionString().ToStdString(), " ");
}
