////////////////////////////////////////////////////////////////////////////////
// Name:      log-none.h
// Purpose:   Declaration of wex::log_none class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
/// This class disables logging, and restores to previous loglevel.
class log_none
{
public:
  /// Default constructor, disables logging.
  log_none();

  /// Enables logging to previous level.
  void enable();

  /// Destructor, enables logging.
  ~log_none();

private:
  const int m_level;
};

}; // namespace wex
