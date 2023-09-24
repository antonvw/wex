////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wex::log class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

#include <bitset>
#include <sstream>

namespace wex
{
/// This class offers logging.
class log
{
public:
  /// The log levels supported.
  /// The order should follow boost::log.
  /// See <boost/log/trivial.hpp>
  enum level_t
  {
    LEVEL_TRACE,   ///< trace level logging (most verbose)
    LEVEL_DEBUG,   ///< debug level
    LEVEL_INFO,    ///< info level
    LEVEL_WARNING, ///< warning level
    LEVEL_ERROR,   ///< error level
    LEVEL_FATAL,   ///< fatal level
    LEVEL_STATUS,  ///< from wxLog
    LEVEL_OFF,     ///< no logging
  };

  /// Static methods.

  /// Initializes logging, and optionally sets logfile.
  /// Should be called before constructing a log object.
  /// The wex::cmdline_imp or wex::app::OnInit takes care of this.
  static void on_init(
    /// loglevel
    level_t loglevel = level_t_def(),
    /// logfile, empty string uses a default logfile
    const std::string& logfile = std::string());

  /// Return current filter log level.
  static auto get_level() { return m_level_filter; }

  /// Returns info for log levels.
  static std::string get_level_info();

  /// Returns default log level.
  static level_t level_t_def();

  /// Returns path for actual logfile used.
  static const std::string path();

  /// Sets filter log level.
  static void set_level(level_t loglevel);

  /// Builds a debug level logger.
  static log debug(const std::string& topic = std::string());

  /// Builds a fatal level logger.
  static log fatal(const std::string& topic = std::string());

  /// Builds a info level logger.
  static log info(const std::string& topic = std::string());

  /// Builds a status level logger.
  static log status(const std::string& topic = std::string());

  /// Builds a trace level logger.
  static log trace(const std::string& topic = std::string());

  /// Builds a warning level logger.
  static log warning(const std::string& topic = std::string());

  /// Other methods.

  /// Default constructor.
  /// This prepares a error level logging.
  explicit log(const std::string& topic = std::string());

  /// Constructor for error level from a std exception.
  explicit log(const std::exception&);

  /// Constructor for error level from a pugi parse result.
  explicit log(const pugi::xml_parse_result&);

  /// Destructor, flushes stringstream to logging.
  ~log();

  /// Supported loggers.

  /// Logs bool according to level.
  log& operator<<(bool);

  /// Logs int according to level.
  log& operator<<(int);

  /// Logs unsigned int according to level.
  log& operator<<(unsigned int);

  /// Logs size_t according to level.
  log& operator<<(size_t);

  /// Logs long according to level.
  log& operator<<(long);

  /// Logs long long according to level.
  log& operator<<(long long);

  /// Logs char according to level.
  log& operator<<(char);

  /// Logs char* according to level.
  log& operator<<(char*);

  /// Logs stringstream according to level.
  log& operator<<(const std::stringstream& ss);

  /// Logs string according to level.
  log& operator<<(const std::string&);

  /// Logs char* according to level.
  log& operator<<(const char*);

  /// Logs char* according to level.
  log& operator<<(const wchar_t*);

  /// Logs a bitset according to level.
  template <std::size_t N> log& operator<<(const std::bitset<N>& b)
  {
    m_ss << S() << b.to_string();
    return *this;
  };

  /// Logs pugi according to level.
  log& operator<<(const pugi::xml_node&);

  /// Logs template class T according to level.
  /// You need a log method inside your template class.
  template <typename T> log& operator<<(const T& t)
  {
    m_ss << S() << t.log().str();
    return *this;
  };

  /// Returns topic and current logging.
  const std::string get() const;

private:
  /// Delegate constructor.
  log(const std::string& topic, level_t level);

  void              flush();
  const std::string S(); // separator

  const std::string m_topic;
  std::stringstream m_ss;
  bool              m_separator{true};
  level_t           m_level;

  static inline bool        m_initialized{false};
  static inline level_t     m_level_filter;
  static inline std::string m_logfile;
};
}; // namespace wex
