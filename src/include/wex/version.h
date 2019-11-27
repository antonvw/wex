////////////////////////////////////////////////////////////////////////////////
// Name:      version.h
// Purpose:   Declaration of class wex::version_info
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

class wxVersionInfo;

namespace wex
{
  /// This class offers version info.
  class version_info
  {
  public:
    /// Constructor.
    version_info(const std::string& name = std::string(),
      int major = 0,
      int minor = 0,
      int micro = 0,
      const std::string& description = std::string(),
      const std::string& copyright = std::string());
         
    /// Returns copyright.
    const std::string copyright() const;
          
    /// Returns description.
    const std::string description() const;
          
    /// Returns string version.
    const std::string get() const;
  private:
    wxVersionInfo m_version;
  };

  /// Returns instantiation of version info class.
  const version_info get_version_info();
};
