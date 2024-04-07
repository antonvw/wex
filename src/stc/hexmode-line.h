////////////////////////////////////////////////////////////////////////////////
// Name:      stc/hexmode-line.h
// Purpose:   Declaration of class hexmode_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/stc/hexmode.h>
#include <wx/event.h>
#include <wx/stc/stc.h>

namespace wex
{
/// Offers a hex mode line.
class hexmode_line
{
public:
  /// Constructor.
  /// Uses current position and line.
  hexmode_line(wex::hexmode* hex);

  /// Constructor.
  /// Specify position or byte offset.
  /// Default assumes you specify a position.
  hexmode_line(hexmode* hex, int pos_or_offset, bool is_position = true);

  /// Erases number and optionally set text.
  bool erase(int count, bool settext = true);

  /// Returns info.
  const std::string info() const;

  /// Inserts text.
  bool insert(const std::string& text);

  /// Returns true if this is the acii field.
  bool is_ascii_field() const;

  /// Returns true if this is the hex field.
  bool is_hex_field() const;

  /// Returns true if this field is readonly.
  bool is_readonly() const;

  /// Returns position of the other field.
  int other_field() const;

  /// Replaces.
  bool replace(char c);
  void replace(const std::string& hex, bool settext);
  void replace_hex(int value);

  /// Sets pos.
  bool set_pos() const;
  void set_pos(const wxKeyEvent& event) const;

private:
  int buffer_index() const;
  int convert(int offset) const { return (m_line_no << 4) + offset; }
  int get_ascii_field() const
  {
    if (m_line[m_column_no] != ' ')
    {
      const int offset = m_column_no / m_hex->each_hex_field();
      return m_start_ascii_field + offset;
    }
    return wxSTC_INVALID_POSITION;
  };

  int get_hex_field() const
  {
    const int offset = m_column_no - m_start_ascii_field;
    return m_hex->each_hex_field() * offset;
  };

  const int m_start_ascii_field;

  std::string m_line;

  int m_column_no, m_line_no;

  wex::hexmode* m_hex;
};

}; // namespace wex

// implementation

inline bool wex::hexmode_line::is_ascii_field() const
{
  return m_column_no >= m_start_ascii_field &&
         m_column_no <
           m_start_ascii_field + static_cast<int>(m_hex->bytes_per_line());
};

inline bool wex::hexmode_line::is_hex_field() const
{
  return m_column_no >= 0 && m_column_no < m_start_ascii_field;
};

inline bool wex::hexmode_line::is_readonly() const
{
  if (is_ascii_field())
  {
    return false;
  }
  else if (is_hex_field())
  {
    if (m_line[m_column_no] != ' ')
    {
      return false;
    }
  }
  return true;
};

inline int wex::hexmode_line::other_field() const
{
  if (is_ascii_field())
  {
    return get_hex_field();
  }
  else if (is_hex_field())
  {
    return get_ascii_field();
  }
  else
  {
    return wxSTC_INVALID_POSITION;
  }
};
