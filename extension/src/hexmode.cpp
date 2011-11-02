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
  , m_STC(stc)
{
}  

bool wxExHexModeLine::AllowReplace(int pos, const wxString& text) const
{
  for (int i = 0; i < text.length(); i++)
  {
    if (IsReadOnly(pos + i))
    {
      return false;
    }
  }
  
  return true;
}

int wxExHexModeLine::BraceMatch(int pos) const
{
  if (pos > start_ascii_field + bytes_per_line)
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (pos >= start_ascii_field)
  {
    const int offset = pos - start_ascii_field;
    int space = 0;

    if (pos == start_ascii_field + bytes_per_line)
    {
      space--;
    }
    else if (pos >= start_ascii_field + bytes_per_line / 2)
    {
      space++;
    }

    return start_hex_field + each_hex_field * offset + space;
  }
  else if (pos >= start_hex_field)
  {
    if (m_Line.GetChar(pos) != ' ')
    {
      int space = 0;

      if (pos >= 
        start_hex_field + 
        space_between_fields + 
        (bytes_per_line * each_hex_field) / 2)
      {
        space++;
      }

      const int offset = (pos - (start_hex_field + space)) / each_hex_field;

      return start_ascii_field + offset;
    }
  }

  return wxSTC_INVALID_POSITION;
}

int wxExHexModeLine::Convert(int offset) const
{
  return atoi(m_Line.Mid(0, start_hex_field)) + offset;
}

int wxExHexModeLine::GetByte(int pos) const
{
  if (pos > start_ascii_field + bytes_per_line)
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (pos >= start_ascii_field)
  {
    return Convert(pos - start_ascii_field);
  }
  else if (pos >= start_hex_field)
  {
    if (m_Line.GetChar(pos) != ' ')
    {
      int space = 0;

      if (pos >= 
        start_hex_field + 
        space_between_fields + 
        (bytes_per_line * each_hex_field) / 2)
      {
        space++;
      }

      return Convert((pos - (start_hex_field + space)) / each_hex_field);
    }
  }

  return wxSTC_INVALID_POSITION;
}
  
bool wxExHexModeLine::IsReadOnly(int pos) const
{
  if (pos >= start_ascii_field)
  {
    return false;
  }
  else if (pos >= start_hex_field)
  {
    if (m_Line.GetChar(pos) != ' ')
    {
      return false;
    }
  }
  
  return true;
}

bool wxExHexModeLine::Replace(int pos, const wxString& text)
{
  if (!AllowReplace(pos, text))
  {
    return false;
  }
  
  for (int i = 0; i < text.length(); i++)
  {
    m_Line[pos + i] = text[i];
    
    m_STC->wxStyledTextCtrl::Replace(
      pos, pos + 1, text[i]);
  }
  
  return true;
}
