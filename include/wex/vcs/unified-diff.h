////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.h
// Purpose:   Declaration of class wex::unified_diff
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/unified-diff.h>

namespace wex
{
class vcs_entry;

namespace factory
{
class frame;
}

/// Offers a class that implements unified_diff report_diff
/// for a vcs entry.
class unified_diff : public factory::unified_diff
{
public:
  /// Constructor.
  unified_diff(
    /// The path (under vcs).
    const path& p,
    /// Provide vcs entry to use the process std out as input.
    const vcs_entry* entry,
    /// Provide frame, that will receive the unified diff callbacks.
    factory::frame* f);

  /// Returns path (from vcs).
  const auto& path_vcs() const { return m_path_vcs; };

private:
  bool report_diff() override;
  void report_diff_finish() override;

  path m_path_vcs;

  factory::frame* m_frame{nullptr};

  const vcs_entry* m_vcs_entry;
  const path       m_path_toplevel;
};
}; // namespace wex
