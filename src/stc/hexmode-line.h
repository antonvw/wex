////////////////////////////////////////////////////////////////////////////////
// Name:      stc/hexmode-line.h
// Purpose:   Declaration of class hexmode_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/hexmode.h>
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

  bool erase(int count, bool settext = true);

  const std::string info() const;

  bool insert(const std::string& text);

  bool is_ascii_field() const
  {
    return m_column_no >= m_start_ascii_field &&
           m_column_no <
             m_start_ascii_field + static_cast<int>(m_hex->bytes_per_line());
  };

  bool is_hex_field() const
  {
    return m_column_no >= 0 && m_column_no < m_start_ascii_field;
  };

  bool is_readonly() const
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

  int other_field() const
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

  bool replace(char c);
  void replace(const std::string& hex, bool settext);
  void replace_hex(int value);

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
