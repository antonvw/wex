////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wxExAddress
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#if wxUSE_GUI

class wxExEx;

/// Offers an address class to be used by vi address ranges.
class WXDLLIMPEXP_BASE wxExAddress : public wxString
{
public:
  /// Constructor for an address.
  wxExAddress(
    /// the ex (or vi) component
    wxExEx* ex, 
    /// the address, being a string containing:
    /// - a normal line number
    /// - a defined marker, 
    ///   like 'x, or '<: begin of selection and '>: end of selection
    /// - $ : last line
    /// - . : current line 
    /// - or a combination of these, using + or -
    /// - or empty, call SetLine afterwards
    const wxString& address = "");
  
  /// Prints this address, with context.
  bool AdjustWindow(const wxString& text) const;
  
  /// Appends text to this address.
  bool Append(const wxString& text) const;
  
  /// Returns false if flags are unsupported.
  bool Flags(const wxString& flags) const;
  
  /// If the line number was set using SetLine, it
  /// returns this line number, otherwise
  /// converts the address to a line number.
  /// This is the vi line number,
  /// so subtract 1 for stc line number.
  /// Returns 0 on error in address. 
  int GetLine() const;
  
  /// Inserts text at this address.
  bool Insert(const wxString& text) const;
  
  /// Marks this address.
  bool MarkerAdd(const wxUniChar& marker) const;
  
  /// Deletes marker (if this address concerns a marker).
  bool MarkerDelete() const;
  
  /// Append text from the specified register at this address, 
  /// default uses yank register.
  bool Put(const char name = '0') const;
  
  /// Read file at this address.
  bool Read(const wxString& arg) const;
  
  /// Sets (vi) line number.
  void SetLine(int line);
  
  /// Shows this address in the ex bar.
  bool Show() const;
private:
  wxExEx* m_Ex;
  int m_Line;
};
#endif // wxUSE_GUI
