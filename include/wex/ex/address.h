////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
class addressrange;
class command_parser;
class ex;

/// Offers an address class to be used by vi address ranges.
class address
{
  friend addressrange;

public:
  /// The kind of address this one is.
  enum class address_t
  {
    IS_BEGIN,  ///< part of range, the begin
    IS_END,    ///< part of range, the end
    IS_SINGLE, ///< not part of range
  };

  /// Static interface.

  /// Returns true if flags are supported (for adjust window,
  /// and addressrange print).
  static bool flags_supported(const std::string& flags);

  /// Other methods.

  /// Default constructor.
  address() = default;

  /// Constructor for an address from a line string.
  explicit address(
    /// the ex (or vi) component
    ex* ex,
    /// the address, being a string containing:
    /// - a normal line number
    /// - a defined marker,
    ///   like 'x, or '<: begin of selection and '>: end of selection
    /// - $ : last line
    /// - . : current line
    /// - or a combination of these, using + or -
    /// - or empty, call set_line afterwards
    const std::string& address = std::string());

  /// Constructor for an address from a line number.
  explicit address(
    /// the ex (or vi) component
    ex* ex,
    /// the address
    int line);

  /// Converts the address to a line number.
  /// This is the vi line number,
  /// so subtract 1 for stc line number.
  /// Returns 0 on error in address.
  /// Default it uses current position to start determine line
  /// number, you can specify another pos as well.
  int get_line(int start_pos = -1) const;

  /// Marks this address.
  bool marker_add(char marker) const;

  /// Deletes marker (if this address concerns a marker).
  bool marker_delete() const;

  /// Parses this address based on command parser.
  /// Returns true if command is valid.
  bool parse(const command_parser& cp);

  /// Supported 1addr commands.
  const std::string regex_commands() const;

  /// Return type of address.
  address_t type() const { return m_type; }

private:
  enum class add_t;

  bool adjust_window(const std::string& text) const;
  bool add(add_t type, const std::string& text) const;
  bool append(const std::string& text) const;
  bool insert(const std::string& text) const;
  bool put(char name = '0') const;
  bool read(const std::string& arg) const;
  void set_line(int line);
  bool write_line_number() const;

  ex*         m_ex{nullptr};
  int         m_line = 0;
  address_t   m_type{address_t::IS_SINGLE};
  std::string m_address; // set by address range
};
}; // namespace wex
