////////////////////////////////////////////////////////////////////////////////
// Name:      version.h
// Purpose:   Declaration of class wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <wx/versioninfo.h>

#include <string>

namespace wex
{
/// This class offers version info.
class version_info
{
public:
  enum
  {
    EXCLUDE_NAME = 0, ///< excludes application name
    EXCLUDE_MICRO,    ///< excludes micro (and revision) number
  };

  /// A typedef containing exclude flags.
  typedef std::bitset<2> exclude_t;

  /// Default constructor.
  explicit version_info(const wxVersionInfo& info = wxVersionInfo());

  /// Returns copyright.
  const std::string copyright() const;

  /// Returns description.
  const std::string description() const;

  /// Returns version as a string depending on type.
  /// If no flags are specified, uses GetVersionString,
  /// otherwise respects flags, and you can exclude
  /// the micro and revision number even if they are non zero.
  const std::string get(exclude_t type = exclude_t()) const;

private:
  wxVersionInfo m_version;
};

/// Returns instantiation of version info class.
const version_info get_version_info();
}; // namespace wex
