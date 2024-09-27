////////////////////////////////////////////////////////////////////////////////
// Name:      block-lines.h
// Purpose:   Declaration of class wex::block_lines
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class ex;

namespace syntax
{
class stc;
} // namespace syntax

/// This class offers a block of lines.
class block_lines
{
public:
  /// Constructor, specify ex component, and start and end line.
  /// If start is -1, then this is an inverse block.
  block_lines(ex* ex, int start = 0, int end = 0);

  /// Assignment operator.
  block_lines& operator=(const block_lines& r);

  /// Spaceship operator.
  auto operator<=>(const block_lines& r) const
  {
    return m_start <=> r.m_start - 1;
  }

  /// Updates end line.
  void end(int line);

  /// Finishes block from other block.
  void finish(const block_lines& block);

  /// Returns addressrange command.
  std::string get_range() const;

  /// Returns true if there is a block is started.
  bool is_available() const;

  /// Logs components.
  void log() const;

  /// Sets indicator based on this block.
  bool set_indicator(const indicator& indicator) const;

  /// Returns block_lines as the first single line from target.
  block_lines single() const;

  /// Return number of lines in the block.
  size_t size() const;

  /// Updates start line.
  void start(int start_line);

  /// Returns block_lines from target.
  block_lines target() const;

private:
  static inline const int LINE_RESET{-2};

  const bool m_is_inverse{false};

  const std::string m_name;

  int m_start{LINE_RESET}, m_end{LINE_RESET};

  ex*          m_ex;
  syntax::stc* m_stc;
};
}; // namespace wex
