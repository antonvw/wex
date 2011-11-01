////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.h
// Purpose:   Declaration of class wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXHEXMODE_H
#define _EXHEXMODE_H

#include <wx/string.h>

const wxFileOffset bytes_per_line = 16;
const wxFileOffset each_hex_field = 3;
const wxFileOffset space_between_fields = 1;
const wxFileOffset start_hex_field = 9;
const wxFileOffset start_ascii_field =
  start_hex_field + each_hex_field * bytes_per_line + space_between_fields;
  
/// Offers a hex mode line.
/*
e.g.:
         <- start_hex_field                               <- start_ascii_field
offset   hex field                                        ascii field
00000000 23 69 6e 63 6c 75 64 65  20 3c 77 78 2f 63 6d 64 #include <wx/cmd
00000010 6c 69 6e 65 2e 68 3e 20  2f 2f 20 66 6f 72 20 77 line.h> // for w
00000020 78 43 6d 64 4c 69 6e 65  50 61 72 73 65 72 0a 23 xCmdLineParser #
         <----------------------------------------------> bytes_per_line
         <-> each_hex_field
                                 <- space_between_fields
                                 <- mid_in_hex_field
*/
class WXDLLIMPEXP_BASE wxExHexModeLine
{
public:
  /// Constructor.
  wxExHexModeLine(const wxString& line);
  
  /// Returns true if you are allowed
  /// to replace text starting at position
  bool AllowReplace(int pos, const wxString& text) const;
  
  /// Returns the position for a matching brace.
  int BraceMatch(int pos) const;
  
  /// Returns the byte no for this position (offset and hex field).
  int GetByte(int pos) const;
  
  /// Gets the line.
  const wxString& GetLine() const {return m_Line;};
  
  /// Returns true if this position refers to a readonly position
  /// (as in the offset field, or on a space).
  bool IsReadOnly(int pos) const;
  
  /// Replaces text at position.
  bool Replace(int pos, const wxString& text);
private:
  int Convert(int offset) const;
  
  wxString m_Line;
};
#endif
