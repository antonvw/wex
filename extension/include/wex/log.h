////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wex::log class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sstream>
#include <pugixml.hpp>
#undef ERROR

namespace wex
{
  class item;
  class listitem;

  /// This class offers logging.
  /// You should give at least one << following one of the
  /// constructors.
  class log
  {
  public:
    /// The log level.
    enum log_level
    {
      INFO,    ///< info
      DEBUG,   ///< debug
      WARNING, ///< warning
      ERROR,   ///< error
      FATAL,   ///< fatal
    };

    /// Default constructor. 
    /// This prepares a logging with default level error.
    log(const std::string& topic = std::string(), log_level level = ERROR);

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

    /// Returns current logging.
    const std::string get() const {return m_ss.str();};

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
  private:
    void Log() const;
    const std::string S(); // separator

    std::stringstream m_ss;
    bool m_Separator {true};
    log_level m_Level {ERROR};
  };
};
