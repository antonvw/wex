////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.cpp
// Purpose:   Implementation of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <atomic>

namespace wex
{
class interruptible_imp
{
public:
  /// Returns true if cancelled.
  bool is_cancelled() const { return m_cancelled; }

  /// Returns true if running.
  bool is_running() const { return m_running; }

  /// Sets members conform cancelling state.
  void cancel();

  /// Sets members conform starting state.
  void start();

  /// Sets members conform stopped state.
  void stop();

private:
  std::atomic<bool> m_cancelled{false};
  std::atomic<bool> m_running{false};
};
}; // namespace wex
