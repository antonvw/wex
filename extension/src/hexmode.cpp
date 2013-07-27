////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/hexmode.h>
#include <wx/extension/stc.h>

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset start_hex_field = 9;
const wxFileOffset start_ascii_field =
  start_hex_field + each_hex_field * bytes_per_line;
  
wxExHexModeLine::wxExHexModeLine(wxExSTC* stc)
  : m_Line(stc->GetCurLine())
  , m_LineNo(stc->GetCurrentLine())
  , m_Index(stc->GetColumn(stc->GetCurrentPos()))
  , m_STC(stc)
{
  wxASSERT(m_STC->HexMode());
}  

wxExHexModeLine::wxExHexModeLine(wxExSTC* stc, 
  int pos_or_offset, bool is_position)
  : m_STC(stc)
{
  wxASSERT(m_STC->HexMode());
  
  if (is_position)
  {
    Set(pos_or_offset != -1 ? pos_or_offset: stc->GetCurrentPos()); 
  }
  else
  {
    char field_offset[start_hex_field];
  
    sprintf(field_offset, "%08lX", (unsigned long)(pos_or_offset & ~0x0f));
  
    if (m_STC->FindNext(field_offset, 0))
    {
      m_LineNo = m_STC->GetCurrentLine();
      m_Line = m_STC->GetLine(m_LineNo);
      m_Index = (pos_or_offset & 0x0f);
      m_STC->SelectNone();
    }
    else
    {
      m_LineNo = -1;
      m_Line.clear();
      m_Index = -1;
    }
  }
}

void wxExHexModeLine::AppendText(const wxCharBuffer& buffer)
{
  wxFileOffset start = m_STC->m_HexBuffer.length();
  
  m_STC->m_HexBuffer += buffer;
  
  wxString text;

  // Allocate space for the string.
  text.Alloc(
    // offset:      (9 + 1 + 1) * length / 16 bytes, 
    (start_hex_field + 1 + 1) * buffer.length() / bytes_per_line + 
    // hex field:   3 * length 
    each_hex_field * buffer.length() + 
    // ascii field: just the length
    buffer.length());

  // Using wxString::Format here asserts (wxWidgets-2.9.1).
  char field_offset[start_hex_field + 1];
  
  for (
    wxFileOffset offset = 0; 
    offset < buffer.length(); 
    offset += bytes_per_line)
  {
    long count = buffer.length() - offset;
    count =
      (bytes_per_line < count ? bytes_per_line : count);

    wxString field_hex, field_ascii;

    for (register wxFileOffset byte = 0; byte < count; byte++)
    {
      const unsigned char c = buffer.data()[offset + byte];

      field_hex += wxString::Format("%02X ", c);
      field_ascii += Printable(c);
    }

    sprintf(field_offset, "%08lX ", (unsigned long)start + (unsigned long)offset);
    
    const wxString field_spaces = wxString(
      ' ', 
      (bytes_per_line - count)* each_hex_field);

    text +=  
      field_offset + 
      field_hex +
      field_spaces +
      field_ascii;
      
    if (buffer.length() - offset > bytes_per_line)
    {
      text += m_STC->GetEOL();
    }
  }

  m_STC->AppendText(text);
}

int wxExHexModeLine::Convert(int offset) const
{
  const wxString address(m_Line.Mid(0, start_hex_field));
  
  long val;
  
  if (sscanf(address.c_str(), "%lx", &val))
  {
    return val + offset;
  }
  
  wxLogError("Cannot convert hex: " + address + " to number");
      
  return 0;
}

int wxExHexModeLine::GetAsciiField() const
{
  if (m_Line.GetChar(m_Index) != ' ')
  {
    const int offset = (m_Index - (start_hex_field)) / each_hex_field;

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
      return Convert((m_Index - (start_hex_field)) / each_hex_field);
    }
  }

  return wxSTC_INVALID_POSITION;
}
  
const wxString wxExHexModeLine::GetInfo() const
{
  if (IsHexField() || IsOffsetField())
  {
    const wxString word = m_STC->GetWordAtPos(m_STC->GetCurrentPos());
    
    if (!word.empty())
    {
      long base16_val;
      const bool base16_ok = word.ToLong(&base16_val, 16);

      if (base16_ok)
      {
        if (IsOffsetField())
        {
          return wxString::Format("%ld", base16_val);
        }
        else
        {
          return wxString::Format("byte: %d %ld", GetByte(), base16_val);
        }
      }
    }
  }
  else if (IsAsciiField())
  {
    return wxString::Format("byte: %d", GetByte());
  }
  
  return wxEmptyString;
}

int wxExHexModeLine::GetHexField() const
{
  const int offset = m_Index - start_ascii_field;

  return start_hex_field + each_hex_field * offset;
}

bool wxExHexModeLine::Goto() const
{
  if (m_LineNo < 0 || m_Index < 0)
  {
    return false;
  }
  
  const int start = m_STC->PositionFromLine(m_LineNo);
 
  m_STC->SetFocus(); 
  m_STC->SetCurrentPos(start + start_ascii_field + m_Index);
  m_STC->SelectNone();
  
  return true;
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

bool wxExHexModeLine::IsOffsetField() const
{
  return 
    m_Index >= 0 &&
    m_Index < start_hex_field;
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

wxUniChar wxExHexModeLine::Printable(unsigned int c) const
{
  // We do not want control chars (\n etc.) to be printed,
  // as that disturbs the hex view field.
  if (isascii(c) && !iscntrl(c))
  {
    return c;
  }
  else
  {
    // If we already defined our own symbol, use that one,
    // otherwise print an ordinary ascii char.
    const int symbol = m_STC->GetControlCharSymbol();
    
    if (symbol == 0)
    {
      return '.';
    }
    else
    {
      return symbol;
    }
  }
}
  
bool wxExHexModeLine::Replace(const wxUniChar& c)
{
  if (IsReadOnly() || m_LineNo < 0 || m_Index < 0)
  {
    return false;
  }
  
  const int byte = GetByte();
  
  if (byte == wxSTC_INVALID_POSITION)
  {
    return false;
  }
  
  const int pos = m_STC->PositionFromLine(m_LineNo);
  
  wxUniChar val = c;
  
  // Because m_HexBuffer is changed, begin and end undo action
  // cannot be used, as these do not operate on the hex buffer.
  
  if (IsAsciiField())
  {
    // replace ascii field with value
    m_STC->wxStyledTextCtrl::Replace(
      pos + m_Index, 
      pos + m_Index + 1, 
      c);
      
    // replace hex field with code
    const wxString code = wxString::Format("%02X", c);
    
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
    
    unsigned int code;
    sscanf(str, "%X", &code);
    
    m_STC->wxStyledTextCtrl::Replace(
      pos + OtherField(), 
      pos + OtherField() + 1, 
      Printable(code));
      
    val = code;
  }
  else
  {
    return false;
  }

  // Only hex buffer, not original,
  // so it can be undone.  
  m_STC->m_HexBuffer[byte] = val;
  
  return true;
}

bool wxExHexModeLine::ReplaceHex(int value)
{
  if (IsReadOnly() || !IsHexField() || m_LineNo < 0 || m_Index < 0)
  {
    return false;
  }
  
  const int byte = GetByte();
  
  if (byte == wxSTC_INVALID_POSITION)
  {
    return false;
  }
  
  const int pos = m_STC->PositionFromLine(m_LineNo);
  
  wxString buffer = wxString::Format("%X", value);
  
  // replace hex field with value
  m_STC->wxStyledTextCtrl::Replace(
    pos + m_Index, 
    pos + m_Index + 2,
    buffer);
        
  // replace ascii field with code
  m_STC->wxStyledTextCtrl::Replace(
    pos + OtherField(), 
    pos + OtherField() + 1, 
    Printable(value));
      
  // Only hex buffer, not original,
  // so it can be undone.  
  m_STC->m_HexBuffer[byte] = value;
  
  return true;
}

void wxExHexModeLine::Set(int pos)
{
  wxASSERT(m_STC->HexMode());
  
  m_LineNo = m_STC->LineFromPosition(pos);
  m_Line = m_STC->GetLine(m_LineNo);
  m_Index = m_STC->GetColumn(pos);
}

void wxExHexModeLine::SetPos(const wxKeyEvent& event)
{
  wxASSERT(m_STC->HexMode());
  
  const int start = m_STC->PositionFromLine(m_LineNo);
  const bool right = (event.GetKeyCode() == WXK_RIGHT);
  const int pos = m_STC->GetCurrentPos();
  
  if (IsOffsetField())
  {
    if (right)
    {
      m_STC->SetCurrentPos(start + start_hex_field);
    }
    else
    {
      if (m_LineNo > 0)
      {
        m_STC->SetCurrentPos(
          m_STC->PositionFromLine(m_LineNo - 1) + start_hex_field);
      }
    }
  }
  else if (IsHexField())
  {
    right ? 
      m_STC->SetCurrentPos(pos + 3):
      m_STC->SetCurrentPos(pos - 3);
      
    if (m_STC->GetCurrentPos() >= start + start_ascii_field)
    {
      m_STC->SetCurrentPos(
        m_STC->PositionFromLine(m_LineNo + 1) + start_hex_field);
    }
    else if (m_STC->GetCurrentPos() < start + start_hex_field)
    {
      if (m_LineNo > 0)
      {
        m_STC->SetCurrentPos(
          m_STC->PositionFromLine(m_LineNo - 1) 
            + start_ascii_field - each_hex_field);
      }
    }
  }
  else
  {
    right ? 
      m_STC->SetCurrentPos(pos + 1):
      m_STC->SetCurrentPos(pos - 1);
      
    if (m_STC->GetCurrentPos() >= start + start_ascii_field + bytes_per_line)
    {
      m_STC->SetCurrentPos(
        m_STC->PositionFromLine(m_LineNo + 1) + start_ascii_field);
    }
    else if (m_STC->GetCurrentPos() < start + start_ascii_field)
    {
      if (m_LineNo > 0)
      {
        m_STC->SetCurrentPos(m_STC->GetLineEndPosition(m_LineNo - 1) - 1);
      }
    }
  }
}
