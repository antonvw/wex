////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.h
// Purpose:   Declaration of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class interruptible_imp;

/// Offers methods to start, stop, cancel things.
class interruptible
{
public:
  /// Cleans up the class.
  static void on_exit();

  /// Initializes the class.
  static void on_init();

  /// Starts the interruptible process.
  /// Returns false if process is already running.
  static bool start();

  /// Stops the interruptible process.
  static void stop();

  /// Cancel interruptible process.
  /// Returns false if process is not running.
  static bool cancel();

  /// Returns true if process was cancelled.
  static bool is_cancelled();

  /// Returns true if process is running.
  static bool is_running();

private:
  static inline interruptible_imp* m_imp{nullptr};
};
}; // namespace wex
