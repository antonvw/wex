////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/test/unittest.h>

// after test include
#include <trompeloeil.hpp>

namespace wex
{
namespace test
{
/// Returns the test path or file in the dir if specified.
const wex::path get_path(
  const std::string& file = std::string(),
  wex::path::log_t        = path::log_t());

/// Offers the unittest based test application,
/// with lib specific init and exit.
/// Your test application should be derived from this class.
class app
  : public wex::app
  , public unittest
{
public:
  // Virtual interface

  /// Prepare environment.
  bool OnInit() override;

  /// Start event loop and start testing.
  int OnRun() override;
};

/// Connects main proc and test app. All tests will start.
///
/// E.g.:
/// int main (int argc, char* argv[])
/// {
///   return wex::test::main(argc, argv, new wex::test::app());
/// }
int main(int argc, char* argv[], app* app);

/// Returns abbreviations.
std::vector<std::pair<std::string, std::string>> get_abbreviations();
}; // namespace test
}; // namespace wex
