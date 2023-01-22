////////////////////////////////////////////////////////////////////////////////
// Name:      doctest.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <doctest.h>
#include <wex/core/app.h>
#include <wex/core/path.h>

namespace wex
{
namespace test
{
/// Offers the doctest based class info.
/// Your test application should be derived from this class.
class doctester
{
public:
  /// Static methods

  /// Returns the test path.
  static path get_path(
    const std::string& file = std::string(),
    path::log_t        type = path::log_t());

  /// Other methods.

  /// Performs doctest init.
  bool on_init(wex::app* app);

  /// Performs doctest activation.
  void on_run(wex::app* app);

  /// Use context when running.
  bool use_context(wex::app* app, int argc, char* argv[]);

private:
  doctest::Context*  m_context{nullptr};
  static inline path m_path;
};
} // namespace test
} // namespace wex
