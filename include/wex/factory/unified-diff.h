////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.h
// Purpose:   Declaration of class wex::factory::unified_diff
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <string>

#include <boost/describe.hpp>

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
  friend class unified_diff_parser;

public:
  /// Type for this diff.
  enum class diff_t
  {
    FIRST,   ///< the first diff from input
    OTHER,   ///< other diffs from input
    LAST,    ///< the last diff from input
    UNKNOWN, ///< no diff found (initial value)
  };

  /// Constructor.
  unified_diff(
    /// Provide input, that is conform unified diff format output.
    std::string input = std::string());

  /// Destructor.
  virtual ~unified_diff() = default;

  // Virtual interface

  /// Do something with a diff.
  virtual bool report_diff() { return true; };

  /// The last diff has been generated, we are finished.
  virtual void report_diff_finish() { ; };

  // Other methods.

  /// Returns number of differences found during parsing.
  size_t differences() const { return m_diffs; };

  /// Returns input.
  const std::string& input() const { return m_input; };

  /// Returns true if this is the first diff of a hunk.
  bool is_first() const { return m_is_first; };

  /// Returns true if this is the last diff of a hunk.
  bool is_last() const { return m_is_last; };

  /// Parses the input.
  /// This routine invokes report_diff methods.
  /// Returns false on error during parsing.
  bool parse();

  /// Returns path from.
  const path& path_from() const { return m_path[0]; };

  /// Returns path to.
  const path& path_to() const { return m_path[1]; };

  /// Returns start number for the from file.
  int range_from_start() const { return m_range[0]; };

  /// Returns count number for the from file.
  int range_from_count() const { return m_range[1]; };

  /// Returns start number for the to file.
  int range_to_start() const { return m_range[2]; };

  /// Returns count number for the to file.
  int range_to_count() const { return m_range[3]; };

  /// Returns text added.
  const std::vector<std::string>& text_added() const { return m_text[1]; };

  /// Returns text removed.
  const std::vector<std::string>& text_removed() const { return m_text[0]; };

  /// Logs this diff as trace logging.
  void trace(const std::string& text) const;

  /// Returns the difference type.
  diff_t type() const { return m_type; };

  /// Copies type from org.
  void type_from(const unified_diff& org) { m_type = org.type(); };

protected:
  std::array<path, 2> m_path;

private:
  std::array<int, 4>                      m_range;
  std::array<std::vector<std::string>, 2> m_text;

  bool m_is_first{true}, m_is_last{true};

  diff_t m_type{diff_t::UNKNOWN};

  size_t m_diffs{0};

  std::string m_input;

  BOOST_DESCRIBE_CLASS(
    unified_diff,
    (),
    (),
    (),
    (m_is_first, m_is_last, m_diffs))

  BOOST_DESCRIBE_NESTED_ENUM(diff_t, FIRST, OTHER, LAST, UNKNOWN)
};
}; // namespace factory
}; // namespace wex
