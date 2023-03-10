////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.h
// Purpose:   Declaration of class wex::addressrange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <vector>

#include <wex/data/substitute.h>
#include <wex/ex/address.h>
#include <wex/syntax/indicator.h>

namespace wex
{
enum class info_message_t;
class command_parser;

namespace factory
{
class process;
} // namespace factory

namespace syntax
{
class stc;
};

/// Offers an address range for vi (ex).
/// - The range is derived from a number of lines,
/// - or by a range string (including visual range for
///   already selected text on the stc component).
/// All methods return false if the range is not ok.
class addressrange
{
public:
  /// Static methods.

  /// Returns substitute data.
  static auto& data() { return m_substitute; }

  /// Constructor for a range from current position
  /// extending with number of lines.
  explicit addressrange(
    /// the ex (or vi) component
    ex* ex,
    /// lines 1 is current line only
    /// lines 0 is illegal
    int lines = 1);

  /// Constructor for a range (including visual range).
  explicit addressrange(
    /// the ex (or vi) component
    ex* ex,
    /// the range, being a string containing:
    /// - .   : current line
    /// - %   : entire document
    /// - *   : current screen visible area
    /// - x,y : range from begin x and end y address.
    /// -     : (empty), the range is empty
    const std::string& range);

  /// Returns begin address.
  auto& begin() const { return m_begin; }

  /// Copies range to destination.
  bool copy(const address& destination) const;

  /// Returns end address.
  auto& end() const { return m_end; }

  /// Deletes range.
  bool erase() const;

  /// Returns find indicator.
  auto& find_indicator() const { return m_find_indicator; }

  /// Returns ex component.
  ex* get_ex() const { return m_ex; }

  /// Is this range ok.
  bool is_ok() const;

  /// Is the begin and address '< and '>.
  bool is_selection() const;

  /// joins range.
  bool join() const;

  /// Parses this addressrange based on command parser.
  /// Returns true if command is valid.
  bool parse(
    /// the command parser
    const command_parser& cp,
    /// extra information in case command failed
    info_message_t& msg);

  /// Supported 2addr commands.
  const std::string regex_commands() const;

  /// Shifts the specified lines to the start of the line.
  bool shift_left() const { return indent(false); }

  /// Shifts the specified lines away from the start of the line.
  bool shift_right() const { return indent(true); }

  /// Yanks range to register, default to yank register.
  bool yank(char name = '0') const;

private:
  typedef std::function<bool(
    /// the command parser
    const command_parser& cp,
    /// extra information in case command failed
    info_message_t& msg)>
    function_t;

  typedef std::vector<std::pair<
    /// the command chars
    const std::string,
    /// command callback
    function_t>>
    commands_t;

  const std::string build_replacement(const std::string& text) const;
  const commands_t  init_commands();

  bool change(const std::string& text) const;
  int confirm(const std::string& pattern, const std::string& replacement) const;
  bool copy(const command_parser& cp);
  bool escape(const std::string& command);
  bool execute(const std::string& reg) const;
  bool general(const address& destination, std::function<bool()> f) const;
  bool global(const command_parser& cp) const;
  bool indent(bool forward = true) const;
  bool move(const address& destination) const;
  bool print(const command_parser& cp);
  bool print(const std::string& flags = std::string()) const;
  bool set(const std::string& begin, const std::string& end);
  void set(address& begin, address& end, int lines) const;
  void set(int begin, int end);
  bool set_range(const std::string& range);
  bool set_selection() const;
  bool set_single(const std::string& line, address& addr);
  bool sort(const std::string& parameters = std::string()) const;
  bool substitute(const command_parser& cp);
  bool write(const command_parser& cp);
  bool write(const std::string& filename) const;
  bool yank(const command_parser& cp);

  static inline data::substitute m_substitute;

  const indicator m_find_indicator{indicator(0)};

  const commands_t m_commands;

  address m_begin, m_end;

  ex*          m_ex;
  syntax::stc* m_stc; // shortcut for m_ex->get_stc()
};
}; // namespace wex
