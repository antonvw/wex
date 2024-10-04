////////////////////////////////////////////////////////////////////////////////
// Name:      block-lines.h
// Purpose:   Declaration of class wex::block_lines
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
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

  /// Returns this block as an addressrange string.
  /// In case the block is not available returns empty string.
  std::string get_range() const;

  /// Returns true if a block is available.
  bool is_available() const;

  /// Logs components.
  void log() const;

  /// Sets indicator based on this block.
  bool set_indicator(const indicator& indicator) const;

  /// Return number of lines in the block.
  size_t size() const;

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
