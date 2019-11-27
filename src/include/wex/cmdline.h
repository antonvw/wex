////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wex::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <string>
#include <utility>
#include <vector>
#include <wex/config.h>

namespace wex
{
  class cmdline_imp;

  /// This class offers a command line parser.
  class cmdline
  {
  public:
    /// Types for command line options.
    enum option_t
    {
      FLOAT,  ///< option is a float value
      INT,    ///< option is a int value
      STRING, ///< option is a string value
    };

    /// Switches: 
    typedef std::vector<std::pair<
      /// vector of switch name, description
      /// - you can specify a flag after name separated by comma
      /// - after description you can also add an implicit value,
      ///   otherwise true is assumed
      /// default a switch toggles, this can be overriden by calling toggle
      const std::vector<std::string>, 
      /// process callback if option is found
      std::function<void(bool on)>>> cmd_switches_t;

    /// Options:
    typedef std::vector<std::pair<
      /// vector of option name, description
      /// - you can specify a flag after name separated by comma
      /// - after description you can also add an implicit value,
      const std::vector<std::string>, 
      /// pair of command line param type and process callback if option is found
      std::pair<
        option_t, 
        std::function<void(const std::any& any)>>>> cmd_options_t;

    /// Params (currently only string value supported): 
    typedef std::pair<
      /// pair of name and description
      const std::pair<const std::string, const std::string>, 
      /// process callback if param is present
      std::function<void(std::vector<std::string> )>> cmd_params_t;

    /// Default constructor, adds standard options.
    cmdline(
      /// switches
      const cmd_switches_t & s = cmd_switches_t(), 
      /// options
      const cmd_options_t & o = cmd_options_t(), 
      /// params
      const cmd_params_t & p = cmd_params_t(),
      /// add standard options
      bool add_standard_options = true,
      /// option prefix for all options
      const std::string& prefix = std::string());

    /// Destructor.
   ~cmdline();

    /// Parses the command line arguments and invokes callbacks.
    /// Returns false if error is found, or exit condition is true.
    bool parse(
      /// argument count
      int ac, 
      /// arguments
      char* av[], 
      /// keep changed values in config
      bool save = false) const;
    
    /// As above, for specified command line string containing all arguments.
    bool parse(
      /// command line
      const std::string& cmdline,
      /// help default goes to specified string
      std::string& help,
      /// keep changed values in config
      bool save = false) const;
    
    /// Return toggle mode.
    static bool toggle() {return m_toggle;};

    /// Sets toggle mode for switches.
    cmdline& toggle(bool value) {m_toggle = value; return *this;};
  private:
    config m_cfg;
    cmdline_imp* m_parser;

    static inline bool 
      m_first {true}, 
      m_toggle {true};
  };
};
