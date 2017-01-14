////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wxExCmdLine class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <sstream> // for tclap!
#include <tclap/CmdLine.h>
#include <wx/any.h>

/// Types for commandline.
enum wxExCmdLineTypes
{
  CMD_LINE_FLOAT,
  CMD_LINE_INT,
  CMD_LINE_STRING,
};

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
    std::pair<wxExCmdLineTypes, std::function<void(const wxAny& any)>>>> CmdOptions;

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
    /// help
    bool helpAndVersion = true);

  /// Destructor.
 ~wxExCmdLine();
  
  /// Parses the specified command line 
  /// (should start with app name, and if empty
  /// the command line from wxTheApp is used).
  bool Parse(const std::string& cmdline = std::string());
private:
  struct wxExCmdLineContent
  {
    wxExCmdLineTypes m_Type;
    std::function<void(const wxAny& any)> m_f; 

    union
    {
      TCLAP::ValueArg<float>* m_val_f; 
      TCLAP::ValueArg<int>* m_val_i; 
      TCLAP::ValueArg<std::string>* m_val_s; 
    } m_tclap_u;
  };

  std::vector<wxExCmdLineContent*> m_Options; 
  std::vector<std::pair<
    TCLAP::UnlabeledMultiArg<std::string>*, 
    std::function<bool(std::vector<std::string>)>>> m_Params; 
  std::vector<std::pair<
    TCLAP::SwitchArg*, 
    std::function<void(bool)>>> m_Switches; 
  
  TCLAP::CmdLine m_CmdLine;
};
