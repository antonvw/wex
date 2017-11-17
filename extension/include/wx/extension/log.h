////////////////////////////////////////////////////////////////////////////////
// Name:      log.h
// Purpose:   Declaration of wxExLog class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <sstream>

enum wxExLogLevel
{
  LEVEL_INFO,
  LEVEL_DEBUG,
  LEVEL_WARNING,
  LEVEL_ERROR,
  LEVEL_FATAL,
};

/// This class offers logging.
class wxExLog
{
public:
  /// Default constructor.
  wxExLog(int level = LEVEL_ERROR);

  /// Logs according to level.
  void Log(const std::stringstream& ss);

  /// Logs exception info according to level.
  void Log(const std::exception& e, const std::stringstream& ss);
private:
  const int m_Level;
};
