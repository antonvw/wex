////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.h
// Purpose:   Declaration of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>

namespace wex
{
/// Offers methods to start, stop things.
/// \dot
/// digraph mode {
///   init        -> idle [style=dotted];
///   idle        -> is_running [label="start"];
///   is_running  -> idle [label="end"];
///  }
/// \enddot
class interruptible
{
public:
  /// Starts the interruptible process.
  /// Returns false if process is already running.
  static bool start();

  /// Stops the interruptible process.
  /// Returns false if process is not running.
  static bool end();

  /// Returns true if process is running.
  static bool is_running();

private:
  static inline std::atomic<bool> m_running{false};
};
}; // namespace wex
