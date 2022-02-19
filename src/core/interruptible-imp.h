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
  /// Returns true if running.
  bool is_running() const { return m_running; }

  /// Sets members conform ending state.
  void end();

  /// Sets members conform starting state.
  void start();

private:
  std::atomic<bool> m_running{false};
};
}; // namespace wex
