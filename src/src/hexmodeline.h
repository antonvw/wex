////////////////////////////////////////////////////////////////////////////////
// Name:      hexmodeline.h
// Purpose:   Declaration of class hexmode_line
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/hexmode.h>

namespace wex
{
  class stc;

  /// Offers a hex mode line.
  class hexmode_line
  {
  public:
    /// Constructor.
    /// Uses current position and line.
    hexmode_line(hexmode* hex);
    
    /// Constructor.
    /// Specify position or byte offset.
    /// Default assumes you specify a position.
    hexmode_line(hexmode* hex, 
      int pos_or_offset, 
      bool is_position = true);

    bool erase(int count, bool settext = true);

    const std::string info() const;

    bool insert(const std::string& text);

    bool is_ascii_field() const {
      return m_ColumnNo >= m_StartAsciiField && 
             m_ColumnNo < m_StartAsciiField + (int)m_Hex->m_BytesPerLine;};

    bool is_hex_field() const {
      return m_ColumnNo >= 0 && m_ColumnNo < m_StartAsciiField;};

    bool is_readonly() const {
      if (is_ascii_field())
      {
        return false;
      }
      else if (is_hex_field())
      {
        if (m_Line[m_ColumnNo] != ' ')
        {
          return false;
        }
      }
      return true;};

    int other_field() const {
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
      }};
    
    bool replace(char c);
    void replace(const std::string& hex, bool settext);
    void replace_hex(int value);

    bool set_pos() const;
    void set_pos(const wxKeyEvent& event) const;
  private:
    int buffer_index() const;
    int convert(int offset) const {
      return (m_LineNo << 4) + offset;};
    int get_ascii_field() const {
      if (m_Line[m_ColumnNo] != ' ')
      {
        const int offset = m_ColumnNo / m_Hex->m_EachHexField;
        return m_StartAsciiField + offset;
      }
      return wxSTC_INVALID_POSITION;};
    int get_hex_field() const {
      const int offset = m_ColumnNo - m_StartAsciiField;
      return m_Hex->m_EachHexField * offset;};
    
    const int m_StartAsciiField;

    std::string m_Line;
    int m_ColumnNo, m_LineNo;
    
    hexmode* m_Hex;
  };

  char printable(unsigned int c, stc* stc);
};

