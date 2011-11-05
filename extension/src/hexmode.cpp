////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/hexmode.h>
#include <wx/extension/stc.h>

wxExHexModeLine::wxExHexModeLine(wxExSTC* stc)
  : m_Line(stc->GetCurLine())
  , m_LineNo(stc->GetCurrentLine())
  , m_Index(stc->GetColumn(stc->GetCurrentPos()))
  , m_STC(stc)
{
}  

wxExHexModeLine::wxExHexModeLine(wxExSTC* stc, int line, int pos)
  : m_STC(stc)
{
  Set(line, pos); 
}

int wxExHexModeLine::Convert(int offset) const
{
  return atoi(m_Line.Mid(0, start_hex_field)) + offset;
}

int wxExHexModeLine::GetAsciiField() const
{
  if (m_Line.GetChar(m_Index) != ' ')
  {
    int space = 0;

    if (m_Index >= 
      start_hex_field + 
      space_between_fields + 
      (bytes_per_line * each_hex_field) / 2)
    {
      space++;
    }

    const int offset = (m_Index - (start_hex_field + space)) / each_hex_field;

    return start_ascii_field + offset;
  }
  
  return wxSTC_INVALID_POSITION;
}

int wxExHexModeLine::GetByte() const
{
  if (m_Index > start_ascii_field + bytes_per_line)
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (m_Index >= start_ascii_field)
  {
    return Convert(m_Index - start_ascii_field);
  }
  else if (m_Index >= start_hex_field)
  {
    if (m_Line.GetChar(m_Index) != ' ')
    {
      int space = 0;

      if (m_Index >= 
        start_hex_field + 
        space_between_fields + 
        (bytes_per_line * each_hex_field) / 2)
      {
        space++;
      }

      return Convert((m_Index - (start_hex_field + space)) / each_hex_field);
    }
  }

  return wxSTC_INVALID_POSITION;
}
  
int wxExHexModeLine::GetHexField() const
{
  const int offset = m_Index - start_ascii_field;
  int space = 0;

  if (m_Index == start_ascii_field + bytes_per_line)
  {
    space--;
  }
  else if (m_Index >= start_ascii_field + bytes_per_line / 2)
  {
    space++;
  }

  return start_hex_field + each_hex_field * offset + space;
}

bool wxExHexModeLine::IsAsciiField() const
{
  return 
    m_Index >= start_ascii_field && 
    m_Index < start_ascii_field + bytes_per_line;
}

bool wxExHexModeLine::IsHexField() const
{
  return 
    m_Index >= start_hex_field &&
    m_Index < start_ascii_field;
}

bool wxExHexModeLine::IsReadOnly() const
{
  if (IsAsciiField())
  {
    return false;
  }
  else if (IsHexField())
  {
    if (m_Line.GetChar(m_Index) != ' ')
    {
      return false;
    }
  }
  
  return true;
}

int wxExHexModeLine::OtherField() const
{
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
  }
}

bool wxExHexModeLine::Replace(const wxUniChar& c)
{
  if (IsReadOnly())
  {
    return false;
  }
  
  const int pos = m_STC->PositionFromLine(m_LineNo);
  
  if (IsAsciiField())
  {
    // replace ascii field with value
    m_STC->wxStyledTextCtrl::Replace(
      pos + m_Index, 
      pos + m_Index + 1, 
      c);
      
    // replace hex field with code
    const wxString code = wxString::Format("%02x", c);
    m_STC->wxStyledTextCtrl::Replace(
      pos + OtherField(), 
      pos + OtherField() + 2, 
      code);
  }
  else if (IsHexField())
  {
    // hex text should be entered.
    if (!isxdigit(c))
    {
      return false;
    }
      
    // replace hex field with value
    m_STC->wxStyledTextCtrl::Replace(
      pos + m_Index, 
      pos + m_Index + 1,
      c);
        
    // replace ascii field with code
    char str[3];
    
    if (m_Line[m_Index + 1] == ' ')
    {
      str[0] = m_Line[m_Index - 1];
      str[1] = c;
    }
    else
    {
      str[0] = c;
      str[1] = m_Line[m_Index];
    }
    
    str[2] = '\0';
    
    int code;
    sscanf(str, "%x", &code);
    
    m_STC->wxStyledTextCtrl::Replace(
      pos + OtherField(), 
      pos + OtherField() + 1, 
      wxUniChar(code));
  }
  else
  {
    return false;
  }
      
  m_STC->m_HexBuffer[GetByte()] = c;
  
  return true;
}

void wxExHexModeLine::Set(int line, int pos)
{
  m_Line = m_STC->GetLine(line);
  m_LineNo = line;
  m_Index = m_STC->GetColumn(pos);
}
