////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wex::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <string>
#include <utility>
#include <vector>

#include <wex/data/cmdline.h>

namespace wex
{
class cmdline_imp;
class config;

/// Offers a command line class. Besides offering the
/// switches, options and params, it also adds standard options:
/// - logfile,D:     sets log file
/// - echo,e:        echo commands (sets is_output)
/// - help,h:        displays usage information and exits
/// - quit,q:        quits after specified number of seconds
/// - version,r:     displays version information and exits
/// - verbose,v:     activates maximum (trace) verbosity
/// - level,V:       activates verbosity down from verbose level
/// - scriptout,w:   script out append (echo to file) (sets get_scriptout)
/// - echo-output,x: echo output commands (process, statusbar) (sets is_echo)
/// - output,X:      output commands append to file (sets get_output)
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
    ///   otherwise false is assumed
    const std::vector<std::string>,
    /// process callback if option is found
    std::function<void(bool on)>>>
    cmd_switches_t;

  /// Options:
  typedef std::vector<std::pair<
    /// vector of option name, description
    /// - you can specify a flag after name separated by comma
    /// - after description you can also add an implicit value,
    const std::vector<std::string>,
    /// pair of command line param type and process callback if option is
    /// found
    std::pair<option_t, std::function<void(const std::any& any)>>>>
    cmd_options_t;

  /// Params (currently only string value supported):
  typedef std::pair<
    /// pair of name and description
    const std::pair<const std::string, const std::string>,
    /// process callback if param is present
    std::function<void(std::vector<std::string>)>>
    cmd_params_t;

  /// Returns output.
  static std::string get_output();

  /// Returns script out.
  static std::string get_scriptout();

  /// Returns echo.
  static bool is_echo();

  /// Returns output.
  static bool is_output();

  /// Default constructor, adds standard options.
  cmdline(
    /// switches
    const cmd_switches_t& s = cmd_switches_t(),
    /// options
    const cmd_options_t& o = cmd_options_t(),
    /// params
    const cmd_params_t& p = cmd_params_t(),
    /// add standard options
    bool add_standard_options = true,
    /// option prefix for all options
    const std::string& prefix = std::string());

  /// Destructor.
  ~cmdline();

  /// Parses the command line arguments and invokes callbacks.
  /// Returns false if error is found, or exit condition is true.
  bool parse(data::cmdline& data);

  /// Parses the command line arguments and invokes callbacks.
  /// Options are specified according to ex :set specification.
  /// [option[=[value]] ...][nooption ...][option? ...][all]
  /// Returns false if option was not found.
  bool parse_set(data::cmdline& data) const;

private:
  void get_all(std::string& help) const;
  bool get_single(const std::vector<std::string>& v, std::string& help) const;
  void init();
  bool set_no_option(const std::vector<std::string>& v, bool save) const;
  bool set_option(const std::vector<std::string>& v, bool save) const;

  const bool m_add_standard_options;

  const cmd_options_t  m_options;
  const cmd_params_t   m_params;
  const cmd_switches_t m_switches;

  config*      m_cfg;
  cmdline_imp* m_parser{nullptr};
};
}; // namespace wex
