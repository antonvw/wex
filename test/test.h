////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <doctest.h>
#include <string>
#include <wex/app.h>
#include <wex/path.h>
#include <wx/platform.h>

namespace wex
{
namespace test
{
/// Returns the test path or file in the dir if specified.
const wex::path get_path(
  const std::string& file = std::string(),
  wex::path::log_t        = path::log_t());

/// Offers the doctest based test application,
/// with lib specific init and exit.
/// Your test application should be derived from this class.
class app : public wex::app
{
public:
  /// Static methods

  /// Returns the test path.
  static path get_path(
    const std::string& file = std::string(),
    path::log_t        type = path::log_t());

  /// Virtual interface

  /// Prepare environment.
  bool OnInit() override;

  /// Start event loop and start testing.
  int OnRun() override;

  /// Other methods

  /// Sets context.
  void set_context(doctest::Context* context);

private:
  doctest::Context*  m_context;
  static inline path m_path;
};

/// Connects main proc and test app. All doctests will start.
///
/// E.g.:
/// int main (int argc, char* argv[])
/// {
///   return wex::test::main(argc, argv, new wex::test::app());
/// }
int main(int argc, char* argv[], app* app);

}; // namespace test
}; // namespace wex

/// Returns abbreviations.
std::vector<std::pair<std::string, std::string>> get_abbreviations();
