////////////////////////////////////////////////////////////////////////////////
// Name:      unified-diff.h
// Purpose:   Declaration of class wex::unified_diff
// Author:    Anton van Wezenbeek
// Copyright: (c) 2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <array>
#include <string>

#include <wex/core/path.h>

namespace wex
{
class frame;
class lexers;

/// Offers a class that parses unified diff process and colours
/// wex::syntax::stc.
class unified_diff
{
public:
  /// Static interface

  /// Sets up unified diff colouring.
  /// Returns false if colours are not present.
  static bool setup(lexers* l);

  /// Constructor.
  unified_diff(
    /// Provide input, that is conform unified diff format output.
    const std::string& input,
    /// Provide frame, that will receive colouring callbacks.
    frame* f);

  /// Parses the entry component.
  /// This routine might invoke colouring methods on wex::frame.
  /// Returns false if setup has not been called, or parsing fails.
  bool parse();

  const auto& range_from_start() const { return m_range[0]; };
  const auto& range_from_count() const { return m_range[1]; };
  const auto& range_to_start() const { return m_range[2]; };
  const auto& range_to_count() const { return m_range[3]; };

  const auto& text_added() const { return m_text[1]; };
  const auto& text_removed() const { return m_text[0]; };

private:
  void colour(const path& p_from, const path& p_to) const;

  std::array<int, 4>                      m_range;
  std::array<std::vector<std::string>, 2> m_text;

  frame* m_frame{nullptr};

  const std::string m_input;

  static inline bool is_setup{false};
};
}; // namespace wex
