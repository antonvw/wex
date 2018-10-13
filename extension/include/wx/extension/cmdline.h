////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wex::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <utility>
#include <vector>
#include <wx/extension/window-data.h>

namespace wex
{
  /// Types for commandline.
  enum cmdline_types
  {
    CMD_LINE_FLOAT,
    CMD_LINE_INT,
    CMD_LINE_STRING,
  };

  class cmdline_option;
  class cmdline_param;
  class cmdline_parser;
  class cmdline_switch;

  /// This class offers a command line parser.
  class cmdline
  {
  public:
    /// Switches: 
    typedef std::vector<std::pair<
      /// vector of switch flag, name, description
      /// - if sizeof first element is greater than one,
      ///   it is supposed to be the name, and a flag is generated,
      ///   starting with 'A'
      /// - after description you can also add a default true value,
      ///   otherwise false is assumed
      const std::vector<std::string>, 
      /// process callback if option is found
      std::function<void(bool)>>> cmd_switches;

    /// Options:
    typedef std::vector<std::pair<
      /// vector of option flag, name, description
      /// - if sizeof first element is greater than one,
      ///   it is supposed to be the name, and a flag is generated
      ///   starting with 'A'
      /// - after description you can also add a default value,
      ///   otherwise 0 is assumed
      const std::vector<std::string>, 
      /// pair of command line param type and process callback if option is found
      std::pair<
        cmdline_types, 
        std::function<void(const std::any& any)>>>> cmd_options;

    /// Params (currently only string value supported): 
    typedef std::pair<
      /// pair of name and description
      const std::pair<const std::string, const std::string>, 
      /// process callback if param is present
      std::function<bool(const std::vector<std::string> &)>> cmd_params;

    /// Constructor, 
    cmdline(
      /// switches
      const cmd_switches & s, 
      /// options
      const cmd_options & o, 
      /// params
      const cmd_params & p = cmd_params(),
      /// message
      const std::string& message = std::string(),
      /// version, if empty use lib version
      const std::string& version = std::string(), 
      /// show help
      bool helpAndVersion = true);

    /// Destructor.
   ~cmdline();

    /// Returns current delimiter.
    char Delimiter() const;
    
    /// Sets delimiter.
    void Delimiter(char c);

    /// Parses the specified command line 
    /// (should start with app name, and if empty
    /// the command line from wxTheApp is used).
    /// Returns false if error is found, or exit
    /// condition is true.
    bool Parse(
      /// command line
      const std::string& cmdline = std::string(),
      /// keep changed values in config
      bool save = false,
      /// delimiter
      const char delimiter = ' ');

    /// Shows current options.
    void ShowOptions(const window_data& data = window_data()) const;
  private:
    std::vector<cmdline_option*> m_Options; 
    std::vector<cmdline_param*> m_Params; 
    std::vector<cmdline_switch*> m_Switches; 
    
    cmdline_parser* m_Parser;
  };
};
