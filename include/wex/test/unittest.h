////////////////////////////////////////////////////////////////////////////////
// Name:      unittest.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/app.h>
#include <wex/core/path.h>

#include <catch.hpp>

namespace Catch
{
class Session;
}

namespace wex
{
namespace test
{
/// Offers the session based class info.
/// Your test application should be derived from this class.
class unittest
{
public:
  // Static methods

  /// Returns the test path.
  static path get_path(
    const std::string& file = std::string(),
    path::log_t        type = path::log_t());

  // Other methods.

  /// Performs session init.
  bool on_init(wex::app* app);

  /// Performs session activation.
  void on_run(wex::app* app);

  /// Start testing.
  bool start(wex::app* app, int argc, char* argv[]);

private:
  Catch::Session* m_session{nullptr};

  bool m_exit_after_test{true};

  static inline path m_path;
};
} // namespace test
} // namespace wex
