////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.h
// Purpose:   Declaration of class wex::factory::unified_diff
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <optional>
#include <string>

#include <wex/core/path.h>

namespace wex
{
namespace factory
{

/// Offers a class that parses a unified diff string and report diffs
/// for a derived class.
/// Context is not expected, you have to create a diff using
/// -U0 (no context).
class unified_diff
{
public:
  /// Constructor.
  unified_diff(
    /// Provide input, that is conform unified diff format output.
    const std::string& input);

  /// Virtual interface

  /// Do something with a diff.
  virtual bool report_diff() { return true; };

  /// The last diff has been generated, we are finished.
  virtual void report_diff_finish() { ; };

  /// Other methods.

  /// Returns true if this is the first diff of a chunk.
  bool is_first() const { return m_is_first; };

  /// Returns true if this is the last diff of a chunk.
  bool is_last() const { return m_is_last; };

  /// Parses the input.
  /// This routine invokes report_diff methods.
  /// Returns number of differences present.
  std::optional<int> parse();

  /// Returns path from.
  const auto& path_from() const { return m_path[0]; };

  /// Returns path to.
  const auto& path_to() const { return m_path[1]; };

  /// Returns start number for the from file.
  const auto& range_from_start() const { return m_range[0]; };

  /// Returns count number for the from file.
  const auto& range_from_count() const { return m_range[1]; };

  /// Returns start number for the to file.
  const auto& range_to_start() const { return m_range[2]; };

  /// Returns count number for the to file.
  const auto& range_to_count() const { return m_range[3]; };

  /// Returns text added.
  const auto& text_added() const { return m_text[1]; };

  /// Returns text removed.
  const auto& text_removed() const { return m_text[0]; };

protected:
  std::array<path, 2> m_path;

private:
  bool parse_header(const std::string& r, const std::string& line, path& p);

  std::array<int, 4>                      m_range;
  std::array<std::vector<std::string>, 2> m_text;

  bool m_is_first{true}, m_is_last{false};

  const std::string m_input;
};
}; // namespace factory
}; // namespace wex
