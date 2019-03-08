////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wex::log class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <sstream>
#include <pugixml.hpp>
#include <wx/chartype.h>
#undef ERROR

namespace wex
{
  class item;
  class listitem;
  class path;

  /// This class offers logging.
  /// You should give at least one << following one of the
  /// constructors.
  class log
  {
  public:
    /// Flags for path logging.
    enum
    {
      STAT_SYNC     = 0, ///< shows 'synchronized' instead of 'modified'
      STAT_FULLPATH = 1  ///< shows file 'fullpath' instead of 'fullname'
    };

    typedef std::bitset<2> status_t;
    
    /// The log level.
    enum level_t
    {
      DEBUG,   ///< debug
      ERROR,   ///< error
      FATAL,   ///< fatal
      STATUS,  ///< status message
      VERBOSE, ///< verbose
      WARNING, ///< warning
    };
    
    /// Initializes logging.
    /// Should be called before constructing a log object.
    static void init(int arc, char** argv);
    
    /// Sets (easy) logging flags.
    static void set_flags(int flags);

    /// Default constructor. 
    /// This prepares a logging with default level error.
    log(
      const std::string& topic = std::string(), 
      level_t level = ERROR, status_t = 0);

    /// Constructor for level error from a std exception.
    log(const std::exception&, level_t = ERROR, status_t = 0);

    /// Constructor for level error from a pugi parse result.
    log(const pugi::xml_parse_result&, level_t = ERROR, status_t = 0);

    /// Constructor for specified log level.
    log(level_t level);

    /// Constructor for specified status.
    log(status_t status);

    /// Destructor, flushes stringstream to logging.
   ~log();

    /// Returns topic and current logging.
    const std::string get() const;

    /// Logs int according to level.
    log& operator<<(int);

    /// Logs size_t according to level.
    log& operator<<(size_t);

    /// Logs long according to level.
    log& operator<<(long);

    /// Logs char according to level.
    log& operator<<(char);

    /// Logs stringstream according to level.
    log& operator<<(const std::stringstream& ss);

    /// Logs string according to level.
    log& operator<<(const std::string&);

    /// Logs char* according to level.
    log& operator<<(const char*);

    /// Logs wxChar* according to level.
    log& operator<<(const wxChar*);

    /// Logs a bitset according to level.
    template<std::size_t N>
    log& operator<<(const std::bitset <N> & b) {
      m_ss << S() << b.to_string();
      return *this;};
    
    /// Logs pugi according to level.
    log& operator<<(const pugi::xml_node&);

    /// Logs item according to level.
    log& operator<<(const item&);

    /// Logs listitem according to level.
    log& operator<<(const listitem&);
    
    /// Logs path info according to level and flags.
    log& operator<<(const path&);
    
    /// Builds a status logger using status.
    static log status(status_t status = 0) {
      return log(std::string(), STATUS, status);};
    
    /// Builds a status logger for an exception.
    static log status(std::exception& e, status_t status = 0) {
      return log(e, STATUS, status);};
    
    /// Builds a status logger using topic and status.
    static log status(const std::string& topic, status_t status = 0) {
      return log(topic, STATUS, status);};

    /// Builds a verbose logger.
    static log verbose(int verbosity = 9) {
      m_verbosity = verbosity; return log(VERBOSE);};
    
    /// Builds a verbose logger from a topic.
    static log verbose(const std::string& topic, int verbosity = 9) {
      m_verbosity = verbosity; return log(topic, VERBOSE);};
    
    /// Builds a verbose logger for an exception.
    static log verbose(std::exception& e, int verbosity = 9) {
      m_verbosity = verbosity; return log(e, VERBOSE);};
  private:
    void flush();
    const std::string S(); // separator

    const std::string m_topic;
    std::stringstream m_ss;
    bool m_separator {true};
    level_t m_level {ERROR};
    status_t m_status {0};
    inline static int m_verbosity {9};
  };
};
