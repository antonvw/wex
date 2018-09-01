////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wxExLog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sstream>
#include <pugixml.hpp>

/// The loglevel.
enum wxExLogLevel
{
  LEVEL_INFO,    ///< info
  LEVEL_DEBUG,   ///< debug
  LEVEL_WARNING, ///< warning
  LEVEL_ERROR,   ///< error
  LEVEL_FATAL,   ///< fatal
};

class wxExItem;
class wxExListItem;

/// This class offers logging.
/// You should give at least one << following one of the
/// constructors.
class wxExLog
{
public:
  /// Default constructor. 
  /// This prepares a logging with default level error.
  wxExLog(
    const std::string& topic = std::string(), 
    wxExLogLevel level = LEVEL_ERROR);

  /// Constructor for specified log level.
  wxExLog(wxExLogLevel level);

  /// Constructor for level error from a std exception.
  wxExLog(const std::exception&);

  /// Constructor for level error fron a pugi exception.
  wxExLog(const pugi::xpath_exception&);

  /// Constructor for level error from a pugi parse result.
  wxExLog(const pugi::xml_parse_result&);

  /// Destructor, flushes stringstream to logging.
 ~wxExLog();

  /// Logs int according to level.
  wxExLog& operator<<(int);

  /// Logs stringstream according to level.
  wxExLog& operator<<(const std::stringstream& ss);

  /// Logs string according to level.
  wxExLog& operator<<(const std::string&);

  /// Logs char* according to level.
  wxExLog& operator<<(const char*);

  /// Logs pugi according to level.
  wxExLog& operator<<(const pugi::xml_node&);

  /// Logs item according to level.
  wxExLog& operator<<(const wxExItem&);

  /// Logs listitem according to level.
  wxExLog& operator<<(const wxExListItem&);

  /// Returns current logging.
  const std::string Get() const {return m_ss.str();};
private:
  void Log() const;
  const std::string S(); // separator

  std::stringstream m_ss;
  bool m_Separator {true};
  wxExLogLevel m_Level {LEVEL_ERROR};
};
