////////////////////////////////////////////////////////////////////////////////
// Name:      block-lines.h
// Purpose:   Declaration of class wex::block_lines
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
class indicator;

namespace syntax
{
class stc;
} // namespace syntax

/// This class offers a block of lines.
class block_lines
{
public:
  /// The block types.
  enum block_t
  {
    MATCH,  ///< normal match block
    INVERSE ///< inverse block
  };

  /// Constructor, specify stc component, start, end line, and type.
  block_lines(
    syntax::stc* s,
    int          start = 0,
    int          end   = 0,
    block_t            = block_t::MATCH);

  /// Spaceship operator.
  auto operator<=>(const block_lines& r) const
  {
    return m_start <=> r.m_start - 1;
  }

  /// Returns end line.
  int end() const { return m_end; };

  /// Updates end line.
  void end(int line);

  /// Finishes block from other block.
  void finish(const block_lines& block);

  /// Returns this block as an addressrange string.
  /// In case the block is not available returns empty string.
  std::string get_range() const;

  /// Returns true if a block is available.
  bool is_available() const;

  /// Logs components.
  void log() const;

  /// Sets indicator based on this block.
  bool set_indicator(const indicator& indicator) const;

  /// Sets lines from other block.
  void set_lines(const block_lines& b)
  {
    m_start = b.m_start;
    m_end   = b.m_end;
  };

  /// Returns number of lines in the block.
  size_t size() const;

  /// Returns start line.
  int start() const { return m_start; };

  /// Updates start line.
  void start(int start_line);

  /// Returns type of block.
  block_t type() const { return m_type; };

private:
  const block_t m_type;

  int m_start{0}, m_end{0};

  syntax::stc* m_stc;
};
}; // namespace wex
