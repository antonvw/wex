////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
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
  enum address_t
  {
    IS_BEGIN,  ///< part of range, the begin
    IS_END,    ///< part of range, the end
    IS_SINGLE, ///< not part of range
  };

  /// Constructor for an address from a line number.
  address(
    /// the ex (or vi) component
    ex* ex,
    /// the address
    int line);

  /// Constructor for an address from a line string.
  address(
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

  /// Prints this address, with context.
  bool adjust_window(const std::string& text) const;

  /// Appends text to this address.
  bool append(const std::string& text) const;

  /// Returns false if flags are unsupported.
  bool flags_supported(const std::string& flags) const;

  /// If the line number was set using set_line, it
  /// returns this line number, otherwise
  /// converts the address to a line number.
  /// This is the vi line number,
  /// so subtract 1 for stc line number.
  /// Returns 0 on error in address.
  /// Default it uses current position to start determine line
  /// number, you can specify another pos as well.
  int get_line(int start_pos = -1) const;

  /// Inserts text at this address.
  bool insert(const std::string& text) const;

  /// Marks this address.
  bool marker_add(char marker) const;

  /// Deletes marker (if this address concerns a marker).
  bool marker_delete() const;

  /// Parse this address based on command parser.
  /// Returns true if command is valid.
  bool parse(const command_parser& cp);

  /// Append text from the specified register at this address,
  /// default uses yank register.
  bool put(char name = '0') const;

  /// Read file at this address.
  bool read(const std::string& arg) const;

  /// Supported 1addr commands.
  const std::string regex_commands() const;

  /// Return type of address.
  address_t type() const { return m_type; }

  /// Shows this address in the ex bar.
  bool write_line_number() const;

private:
  /// Sets (vi) line number.
  void set_line(int line);

  ex*         m_ex;
  int         m_line = 0;
  address_t   m_type{IS_SINGLE};
  std::string m_address; // set by address range
};
}; // namespace wex
