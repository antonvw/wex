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
#include <wx/stc/stc.h>

wxExHexModeLine::wxExHexModeLine(const wxString& line)
  : m_Line(line)
{
}  

int wxExHexModeLine::BraceMatch(int pos) const
{
  if (pos >= start_ascii_field)
  {
    const int offset = pos - start_ascii_field;
    int space = 0;

    if (pos > start_ascii_field + bytes_per_line)
    {
      return wxSTC_INVALID_POSITION;
    }
    else if (pos == start_ascii_field + bytes_per_line)
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
