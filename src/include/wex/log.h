////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wex::log class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <sstream>
#include <pugixml.hpp>
#include <wx/chartype.h>
#include <wx/string.h>
#undef ERROR

namespace wex
{
  /// This class offers logging.
  /// You should give at least one << following one of the
  /// constructors.
  class log
  {
  public:
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
    
    /// Static methods.
    
    /// Initializes logging.
    /// Should be called before constructing a log object.
    static void init(int arc, char** argv);
    
    /// Builds a status logger.
    static log status() {
      return log(std::string(), STATUS);};
    
    /// Builds a status logger for an exception.
    static log status(std::exception& e) {return log(e, STATUS);};
    
    /// Builds a status logger using topic and status.
    static log status(const std::string& topic) {return log(topic, STATUS);};

    /// Builds a verbose logger.
    static log verbose(int verbosity = 9) {
      m_verbosity = verbosity; return log(VERBOSE);};
    
    /// Builds a verbose logger from a topic.
    static log verbose(const std::string& topic, int verbosity = 9) {
      m_verbosity = verbosity; return log(topic, VERBOSE);};
    
    /// Builds a verbose logger for an exception.
    static log verbose(std::exception& e, int verbosity = 9) {
      m_verbosity = verbosity; return log(e, VERBOSE);};
    
    /// Builds a warning logger.
    static log warning(int verbosity = 9) {
      m_verbosity = verbosity; return log(WARNING);};
    
    /// Builds a warning logger from a topic.
    static log warning(const std::string& topic, int verbosity = 9) {
      m_verbosity = verbosity; return log(topic, WARNING);};
    
    /// Builds a warning logger for an exception.
    static log warning(std::exception& e, int verbosity = 9) {
      m_verbosity = verbosity; return log(e, WARNING);};
    
    /// Other methods.

    /// Default constructor. 
    /// This prepares a logging with default level error.
    log(const std::string& topic = std::string(), level_t level = ERROR);

    /// Constructor for level error from a std exception.
    log(const std::exception&, level_t = ERROR);

    /// Constructor for level error from a pugi parse result.
    log(const pugi::xml_parse_result&, level_t = ERROR);

    /// Constructor for specified log level.
    log(level_t level);

    /// Destructor, flushes stringstream to logging.
   ~log();

    /// Returns topic and current logging.
    const std::string get() const;
    
    /// Supported loggers.

    /// Logs bool according to level.
    log& operator<<(bool);

    /// Logs int according to level.
    log& operator<<(int);

    /// Logs size_t according to level.
    log& operator<<(size_t);

    /// Logs long according to level.
    log& operator<<(long);

    /// Logs long long according to level.
    log& operator<<(long long);

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

    /// Logs wxString according to level.
    log& operator<<(const wxString&);

    /// Logs a bitset according to level.
    template<std::size_t N>
    log& operator<<(const std::bitset <N> & b) {
      m_ss << S() << b.to_string();
      return *this;};
    
    /// Logs pugi according to level.
    log& operator<<(const pugi::xml_node&);

    /// Logs template class T according to level.
    /// You need a log method inside your template class.
    template <typename T>
    log& operator<<(const T & t) {
      m_ss << S() << t.log().str();
      return *this;};
  private:
    void flush();
    const std::string S(); // separator

    const std::string m_topic;
    std::stringstream m_ss;
    bool m_separator {true};
    level_t m_level {ERROR};
    inline static int m_verbosity {1};
  };
};
