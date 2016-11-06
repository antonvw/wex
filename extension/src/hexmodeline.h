////////////////////////////////////////////////////////////////////////////////
// Name:      hexmodeline.h
// Purpose:   Declaration of class wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/hexmode.h>
#include <wx/extension/stc.h>

char Printable(unsigned int c, wxExSTC* stc);

/// Offers a hex mode line.
class WXDLLIMPEXP_BASE wxExHexModeLine
{
public:
  /// Constructor.
  /// Uses current position and line.
  wxExHexModeLine(wxExHexMode* hex);
  
  /// Constructor.
  /// Specify position or byte offset.
  /// Default assumes you specify a position.
  wxExHexModeLine(wxExHexMode* hex, 
    int pos_or_offset, 
    bool is_position = true);

  bool Delete(int count, bool settext = true);
  const std::string GetInfo() const;
  bool Goto() const;
  bool Insert(const std::string& text);
  bool IsAsciiField() const {
    return m_ColumnNo >= m_StartAsciiField && 
           m_ColumnNo < m_StartAsciiField + m_Hex->m_BytesPerLine;};
  bool IsHexField() const {
    return m_ColumnNo >= 0 && m_ColumnNo < m_StartAsciiField;};
  bool IsReadOnly() const {
    if (IsAsciiField())
    {
      return false;
    }
    else if (IsHexField())
    {
      if (m_Line[m_ColumnNo] != ' ')
      {
        return false;
      }
    }
    return true;};
  int OtherField() const {
    if (IsAsciiField())
    {
      return GetHexField();
    }
    else if (IsHexField())
    {
      return GetAsciiField();
    }
    else
    {
      return wxSTC_INVALID_POSITION;
    }};
  
  bool Replace(char c);
  void Replace(const std::string& hex, bool settext);
  void ReplaceHex(int value);
  void SetPos(const wxKeyEvent& event);
private:
  int Convert(int offset) const {
    return (m_LineNo << 4) + offset;};
  int GetAsciiField() const {
    if (m_Line[m_ColumnNo] != ' ')
    {
      const int offset = m_ColumnNo / m_Hex->m_EachHexField;
      return m_StartAsciiField + offset;
    }
    return wxSTC_INVALID_POSITION;};
  int GetBufferIndex() const;
  int GetHexField() const {
    const int offset = m_ColumnNo - m_StartAsciiField;
    return m_Hex->m_EachHexField * offset;};
  
  const wxFileOffset m_StartAsciiField;

  std::string m_Line;
  int m_ColumnNo, m_LineNo;
  
  wxExHexMode* m_Hex;
};
