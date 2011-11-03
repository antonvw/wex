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
  , m_Index(stc->GetColumn(stc->GetCurrentPos()))
  , m_STC(stc)
{
}  

 wxExHexModeLine::wxExHexModeLine(wxExSTC* stc, int line, int pos)
  : m_Stc(stc)
{
  Set(line, pos); 
}

bool wxExHexModeLine::AllowReplace(const wxString& text) const
{
  for (int i = 0; i < text.length(); i++)
  {
    if (IsReadOnly(m_Index + i))
    {
      return false;
    }
  }
  
  return true;
}

int wxExHexModeLine::BraceMatch() const
{
  if (m_Index > start_ascii_field + bytes_per_line)
  {
    return wxSTC_INVALID_POSITION;
  }
  else if (m_Index >= start_ascii_field)
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

      const int offset = (m_Index - (start_hex_field + space)) / each_hex_field;

      return start_ascii_field + offset;
    }
  }

  return wxSTC_INVALID_POSITION;
}

int wxExHexModeLine::Convert(int offset) const
{
  return atoi(m_Line.Mid(0, start_hex_field)) + offset;
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
  
bool wxExHexModeLine::IsReadOnly(int m_Index) const
{
  if (m_Index >= start_ascii_field)
  {
    return false;
  }
  else if (m_Index >= start_hex_field)
  {
    if (m_Line.GetChar(m_Index) != ' ')
    {
      return false;
    }
  }
  
  return true;
}

bool wxExHexModeLine::Replace(const wxString& text)
{
  if (!AllowReplace(m_Index, text))
  {
    return false;
  }
  
  for (int i = 0; i < text.length(); i++)
  {
    m_Line[m_Index + i] = text[i];
    
    m_STC->wxStyledTextCtrl::Replace(
      m_Index, m_Index + 1, text[i]);
  }
  
  return true;
}

void wxExHexModeLine::Set(int line, int pos)
{
  m_Line = line;
  m_Index = m_STC->GetColumn(pos);
}
