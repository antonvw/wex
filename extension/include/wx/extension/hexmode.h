////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.h
// Purpose:   Declaration of class wxExHexModeLine
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXHEXMODE_H
#define _EXHEXMODE_H

#include <wx/string.h>

class wxExSTC;
  
/// Offers a hex mode line.
/*
e.g.:
         <- start_hex_field                              <- start_ascii_field
offset   hex field                                       ascii field
00000000 23 69 6e 63 6c 75 64 65 20 3c 77 78 2f 63 6d 64 #include <wx/cmd
00000010 6c 69 6e 65 2e 68 3e 20 2f 2f 20 66 6f 72 20 77 line.h> // for w
00000020 78 43 6d 64 4c 69 6e 65 50 61 72 73 65 72 0a 23 xCmdLineParser #
         <---------------------------------------------> bytes_per_line
         <-> each_hex_field
*/
class WXDLLIMPEXP_BASE wxExHexModeLine
{
public:
  /// Constructor.
  /// Uses current position and line.
  wxExHexModeLine(wxExSTC* stc);
  
  /// Constructor.
  /// Specify position or byte offset.
  /// Default assumes you specify a position.
  wxExHexModeLine(wxExSTC* stc, 
    int pos_or_offset, 
    bool is_position = true);

  /// Appends hex mode lines to stc  component.  
  void AppendText(const wxCharBuffer& buffer);
  
  /// Returns info about current index,
  /// depending on which field is current.
  const wxString GetInfo() const;
  
  /// Goes to position in stc component as
  /// implied by current line and index.
  bool Goto() const;
  
  /// Returns true if current index is within ascii field.
  bool IsAsciiField() const;
  
  /// Returns true if current index is within hex field.
  bool IsHexField() const;
  
  /// Returns true if current index is within offset field.
  bool IsOffsetField() const;
  
  /// Returns true if current index refers to a readonly position in current line.
  /// (as in the offset field, or on a space).
  bool IsReadOnly() const;

  /// If on ascii field, return index for hex field,
  /// if on hex field, return index for ascii field,
  /// if on offset field or invalid field, returns wxSTC_INVALID_POSITION.
  int OtherField() const;
  
  /// Replaces current line at current index with char for
  /// both ascii and hex field. Also updates the hex buffer from stc.
  bool Replace(const wxUniChar& c);
  
  /// Replaces current line at current and next index (the hex field) 
  /// with value for both ascii and hex field. 
  /// Also updates the hex buffer from stc.
  bool ReplaceHex(int value);
  
  /// Sets line and index from specified position on stc component.
  void Set(int pos);
  
  /// Sets caret pos on stc, depending on event and 
  /// where we are.
  void SetPos(const wxKeyEvent& event);
private:
  int Convert(int offset) const;
  
  int GetAsciiField() const;
  /// Returns the byte no for this position (offset and hex field).
  int GetByte() const;
  int GetHexField() const;
  
  wxUniChar Printable(unsigned int c) const;
  
  wxString m_Line;
  int m_LineNo;
  int m_Index;
  wxExSTC* m_STC;
};
#endif
