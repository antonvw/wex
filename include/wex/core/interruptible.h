////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.h
// Purpose:   Declaration of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
  /// Offers methods to start, stop things.
  class interruptible
  {
  public:
    /// Cancel interruptible process.
    /// Returns false if process was not running.
    static bool cancel();

    /// Check whether process was cancelled.
    static bool is_cancelled() { return m_cancelled; }

    /// Is process running.
    static bool is_running() { return m_running; }

    /// Starts interruptible process.
    /// Returns false if process is already running.
    static bool start();

    /// Stops interruptible process.
    static void stop();

  private:
    static inline bool m_cancelled{false}, m_running{false};
  };
}; // namespace wex
