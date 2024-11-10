////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.h
// Purpose:   Declaration of class wex::unified_diff
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
class vcs_entry;

namespace factory
{
class frame;
}

/// Offers a class that parses a unified diff string and invokes
/// wex::frame callbacks.
/// Context is not expected, you have to create a diff using
/// -U0 (no context).
class unified_diff
{
public:
  /// Constructor.
  unified_diff(
    /// Provide input, that is conform unified diff format output.
    const std::string& input);

  /// Constructor.
  unified_diff(
    /// The path (under vcs).
    const path& p,
    /// Provide vcs entry to use the process std out as input.
    const vcs_entry* entry,
    /// Provide frame, that will receive the unified diff callbacks.
    factory::frame* f);

  /// Parses the input.
  /// This routine might invoke callback methods on wex::frame.
  /// Returns number of differences present.
  std::optional<int> parse();

  /// Returns path from.
  const auto& path_from() const { return m_path[0]; };

  /// Returns path to.
  const auto& path_to() const { return m_path[1]; };

  /// Returns path (from vcs).
  const auto& path_vcs() const { return m_path_vcs; };

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

private:
  bool parse_header(const std::string& r, const std::string& line, path& p);

  std::array<int, 4>                      m_range;
  std::array<path, 2>                     m_path;
  std::array<std::vector<std::string>, 2> m_text;

  path m_path_vcs;

  const vcs_entry* m_vcs_entry{nullptr};
  factory::frame*  m_frame{nullptr};

  const path        m_path_toplevel;
  const std::string m_input;
};
}; // namespace wex
