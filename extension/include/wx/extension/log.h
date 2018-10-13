////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wex::log class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sstream>
#include <pugixml.hpp>

namespace wex
{
  class item;
  class listitem;

  /// The log level.
  enum log_level
  {
    LEVEL_INFO,    ///< info
    LEVEL_DEBUG,   ///< debug
    LEVEL_WARNING, ///< warning
    LEVEL_ERROR,   ///< error
    LEVEL_FATAL,   ///< fatal
  };

  /// This class offers logging.
  /// You should give at least one << following one of the
  /// constructors.
  class log
  {
  public:
    /// Default constructor. 
    /// This prepares a logging with default level error.
    log(const std::string& topic = std::string(), 
        log_level level = LEVEL_ERROR);

    /// Constructor for specified log level.
    log(log_level level);

    /// Constructor for level error from a std exception.
    log(const std::exception&);

    /// Constructor for level error fron a pugi exception.
    log(const pugi::xpath_exception&);

    /// Constructor for level error from a pugi parse result.
    log(const pugi::xml_parse_result&);

    /// Destructor, flushes stringstream to logging.
   ~log();

    /// Logs int according to level.
    log& operator<<(int);

    /// Logs stringstream according to level.
    log& operator<<(const std::stringstream& ss);

    /// Logs string according to level.
    log& operator<<(const std::string&);

    /// Logs char* according to level.
    log& operator<<(const char*);

    /// Logs pugi according to level.
    log& operator<<(const pugi::xml_node&);

    /// Logs item according to level.
    log& operator<<(const item&);

    /// Logs listitem according to level.
    log& operator<<(const listitem&);

    /// Returns current logging.
    const std::string Get() const {return m_ss.str();};
  private:
    void Log() const;
    const std::string S(); // separator

    std::stringstream m_ss;
    bool m_Separator {true};
    log_level m_Level {LEVEL_ERROR};
  };
};
