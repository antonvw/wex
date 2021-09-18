////////////////////////////////////////////////////////////////////////////////
// Name:      factory/hexmode.h
// Purpose:   Declaration of class wex::factory::hexmode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

import<string>;

namespace wex
{
namespace factory
{
class stc;

/// Offers a hex mode view to stc component
class hexmode
{
public:
  /// Constructor.
  hexmode(
    /// stc to view in hexmode
    factory::stc* stc,
    /// @code
    /// hex field                                       ascii field
    /// 23 69 6e 63 6c 75 64 65 20 3c 77 78 2f 63 6d 64 #include <wx/cmd
    /// 6c 69 6e 65 2e 68 3e 20 2f 2f 20 66 6f 72 20 77 line.h> // for w
    /// 78 43 6d 64 4c 69 6e 65 50 61 72 73 65 72 0a 23 xCmdLineParser #
    /// <---------------------------------------------> bytes per line
    /// @endcode
    size_t bytes_per_line = 16);

  /// Destructor.
  virtual ~hexmode() = default;

  /// Returns number of bytes per line.
  auto bytes_per_line() const { return m_bytes_per_line; }

  /// Returns each hex field.
  auto each_hex_field() const { return m_each_hex_field; }

  /// Returns stc component.
  auto* get_stc() { return m_stc; }

  /// Returns true if hex mode is on.
  bool is_active() const { return m_is_active; }

  /// Converts text into hex lines.
  const std::string lines(const std::string& text) const;

  /// Make hex mode active.
  void make_active(bool on) { m_is_active = on; }

  /// Returns a printable char.
  char printable(unsigned int c) const;

protected:
  /// Actions to be done when activated.
  virtual void activate();

  /// Actions to be done when deactivated.
  virtual void deactivate();

private:
  /// Converts text into hex line.
  const std::string line(
    /// the text
    const std::string& text,
    /// the offset
    size_t offset) const;

  const size_t m_bytes_per_line, m_each_hex_field;

  bool m_is_active{false};

  factory::stc* m_stc;
};
}; // namespace factory
}; // namespace wex
