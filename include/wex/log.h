////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wex::log class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <pugixml.hpp>
#include <sstream>
#undef ERROR

namespace wex
{
  /// This class offers logging.
  /// You should give at least one << following one of the
  /// constructors.
  class log
  {
  public:
    /// Static methods.

    /// Initializes logging.
    /// Should be called before constructing a log object.
    static void init(int arc, char** argv);

    /// Builds a debug logger.
    static log debug(const std::string& topic = std::string());

    /// Builds a info logger.
    static log info(const std::string& topic = std::string());

    /// Builds a status logger.
    static log status(const std::string& topic = std::string());

    /// Builds a trace logger.
    static log trace(const std::string& topic = std::string());

    /// Builds a warning logger.
    static log warning(const std::string& topic = std::string());

    /// Other methods.

    /// Default constructor.
    /// This prepares a logging error.
    log(const std::string& topic = std::string());

    /// Constructor for level error from a std exception.
    log(const std::exception&);

    /// Constructor for level error from a pugi parse result.
    log(const pugi::xml_parse_result&);

    /// Destructor, flushes stringstream to logging.
    ~log();

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
    /// The log level.
    enum class level_t;

    /// Constructor for topic and specified log level.
    log(const std::string& topic, level_t level);

    void              flush();
    const std::string S(); // separator

    const std::string  m_topic;
    std::stringstream  m_ss;
    std::wstringstream m_wss;
    bool               m_separator{true};
    level_t            m_level;
  };
}; // namespace wex
