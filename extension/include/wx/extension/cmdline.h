////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wxExCmdLineParser class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <wx/any.h>
#include <wx/cmdline.h>

/// This class offers a command line parser.
class WXDLLIMPEXP_BASE wxExCmdLineParser
{
  public:
    /// Switches: 
    typedef std::vector<std::pair<
      /// pair of option name and description
      std::pair<std::string, std::string>, 
      /// pair of flags and process callback if option is found
      std::pair<
        /// flags:
        /// - wxCMD_LINE_OPTION_MANDATORY 	
        /// - wxCMD_LINE_OPTION_HELP 	
        /// - wxCMD_LINE_SWITCH_NEGATABLE 
        int, 
        std::function<void(bool)>>>> CmdSwitches;

    /// Options: 
    typedef std::vector<std::pair<
      /// pair of option name and description
      std::pair<std::string, std::string>, 
      /// pair of command line param type and process callback if option is found
      std::pair<
        /// type:
        /// - wxCMD_LINE_VAL_STRING 	
        /// - wxCMD_LINE_VAL_NUMBER 	
        /// - wxCMD_LINE_VAL_DATE 	
        /// - wxCMD_LINE_VAL_DOUBLE 	
        /// - wxCMD_LINE_VAL_NONE 	
        wxCmdLineParamType, 
        std::function<void(wxAny)>>>> CmdOptions;

    /// Params (currently only string value supported): 
    typedef std::vector<std::pair<
      /// description
      std::string, 
      /// pair of flags and process callback if param is present
      std::pair<
        /// flags:
        /// - wxCMD_LINE_PARAM_OPTIONAL 	
        /// - wxCMD_LINE_PARAM_MULTIPLE 	
        /// - wxCMD_LINE_NEEDS_SEPARATOR 	
        int, 
        std::function<void(std::vector<std::string> &)>>>> CmdParams;
  
    /// Contructor, 
    /// Default (i.e. if flags are just 0), options are optional.
    /// If you specify an option name of max short_option_size, it is considered a
    /// short option, if you specify more chars, it is considered
    /// a long option.
    wxExCmdLineParser(
      /// the command line to be parsed
      const std::string& cmdline, 
      /// switches
      const CmdSwitches & s, 
      /// options
      const CmdOptions & o, 
      /// params
      const CmdParams & p = CmdParams(),
      /// default size for short options
      size_t short_option_size = 2);

    /// Constructor using command line from wxTheApp.
    wxExCmdLineParser(
      const CmdSwitches & s, const CmdOptions & o, const CmdParams & p = CmdParams());
    
    /// Parses the command line and invokes callbacks, returns:
    /// - -1 for help, 
    /// - 0 if ok, 
    /// - a positive value if an error occurred. 
    int Parse(bool giveUsage = true);
private:
  const CmdOptions m_Options; 
  const CmdParams m_Params; 
  const CmdSwitches m_Switches; 
  
  wxCmdLineParser m_Parser;
};
