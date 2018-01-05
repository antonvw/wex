////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wxExCmdLine class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <utility>
#include <vector>

/// Types for commandline.
enum wxExCmdLineTypes
{
  CMD_LINE_FLOAT,
  CMD_LINE_INT,
  CMD_LINE_STRING,
};

class wxExCmdLineOption;
class wxExCmdLineParam;
class wxExCmdLineParser;
class wxExCmdLineSwitch;

/// This class offers a command line parser.
class WXDLLIMPEXP_BASE wxExCmdLine
{
public:
  /// Switches: 
  typedef std::vector<std::pair<
    /// tuple of option flag, name, description
    const std::tuple<const std::string, const std::string, const std::string>, 
    /// process callback if option is found
    std::function<void(bool)>>> CmdSwitches;

  /// Options:
  typedef std::vector<std::pair<
    /// tuple of option flag, name and description
    const std::tuple<const std::string, const std::string, const std::string>, 
    /// pair of command line param type and process callback if option is found
    std::pair<wxExCmdLineTypes, std::function<void(const std::any& any)>>>> CmdOptions;

  /// Params (currently only string value supported): 
  typedef std::pair<
    /// pair of name and description
    const std::pair<const std::string, const std::string>, 
    /// process callback if param is present
    std::function<bool(const std::vector<std::string> &)>> CmdParams;

  /// Constructor, 
  wxExCmdLine(
    /// switches
    const CmdSwitches & s, 
    /// options
    const CmdOptions & o, 
    /// params
    const CmdParams & p = CmdParams(),
    /// message
    const std::string& message = std::string(),
    /// delimiter
    const char delimiter = ' ', 
    /// version, if empty use wxExtension version
    const std::string& version = std::string(), 
    /// show help
    bool helpAndVersion = true);

  /// Destructor.
 ~wxExCmdLine();
  
  /// Parses the specified command line 
  /// (should start with app name, and if empty
  /// the command line from wxTheApp is used).
  bool Parse(
    /// command line
    const std::string& cmdline = std::string(),
    /// allow to toggle between switches
    bool toggle = false);
private:
  std::vector<wxExCmdLineOption*> m_Options; 
  std::vector<wxExCmdLineParam*> m_Params; 
  std::vector<wxExCmdLineSwitch*> m_Switches; 
  
  wxExCmdLineParser* m_Parser;
};
