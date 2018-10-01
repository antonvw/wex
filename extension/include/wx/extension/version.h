////////////////////////////////////////////////////////////////////////////////
// Name:      version.h
// Purpose:   Declaration of class wxExVersionInfo
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

class wxVersionInfo;

/// This class offers version info.
class wxExVersionInfo
{
public:
  /// Constructor.
  wxExVersionInfo(const std::string& name = std::string(),
    int major = 0,
    int minor = 0,
    int micro = 0,
    const std::string& description = std::string(),
    const std::string& copyright = std::string());
       
  /// Returns copyright.
  const std::string Copyright() const;
        
  /// Returns description.
  const std::string Description() const;
        
  /// Returns string version.
  const std::string Get() const;
private:
  wxVersionInfo m_version;
};

/// Returns instantiation of version info class.
const wxExVersionInfo wxExGetVersionInfo();
