////////////////////////////////////////////////////////////////////////////////
// Name:      version.h
// Purpose:   Declaration of class wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/versioninfo.h>

import<sstream>;
import<string>;

namespace wex
{
/// This class offers version info.
class version_info
{
public:
  /// Default constructor.
  explicit version_info(const wxVersionInfo& info = wxVersionInfo());

  /// Returns copyright.
  const std::string copyright() const;

  /// Returns description.
  const std::string description() const;

  /// Returns external libraries used.
  const std::stringstream external_libraries() const;

  /// Returns string version.
  const std::string get() const;

private:
  const wxVersionInfo m_version;
};

/// Returns instantiation of version info class.
const version_info get_version_info();
}; // namespace wex
