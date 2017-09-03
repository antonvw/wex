////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wxExAddress
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#if wxUSE_GUI

class wxExEx;
class wxExAddressRange;

/// Offers an address class to be used by vi address ranges.
class WXDLLIMPEXP_BASE wxExAddress
{
  friend wxExAddressRange;
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
    const std::string& address = std::string())
    : m_Address(address)
    , m_Ex(ex) {;};
  
  /// Prints this address, with context.
  bool AdjustWindow(const std::string& text) const;
  
  /// Appends text to this address.
  bool Append(const std::string& text) const;
  
  /// Returns false if flags are unsupported.
  bool Flags(const std::string& flags) const;
  
  /// Returns address as specified during construction.
  const auto & Get() const {return m_Address;};
  
  /// If the line number was set using SetLine, it
  /// returns this line number, otherwise
  /// converts the address to a line number.
  /// This is the vi line number,
  /// so subtract 1 for stc line number.
  /// Returns 0 on error in address. 
  int GetLine() const;
  
  /// Inserts text at this address.
  bool Insert(const std::string& text) const;
  
  /// Marks this address.
  bool MarkerAdd(char marker) const;
  
  /// Deletes marker (if this address concerns a marker).
  bool MarkerDelete() const;
  
  /// Append text from the specified register at this address, 
  /// default uses yank register.
  bool Put(char name = '0') const;
  
  /// Read file at this address.
  bool Read(const std::string& arg) const;
  
  /// Shows this address in the ex bar.
  bool WriteLineNumber() const;
private:
  /// Sets (vi) line number.
  void SetLine(int line);
  
  wxExEx* m_Ex;
  int m_Line = 0;
  
  std::string m_Address; // set by address range
};
#endif // wxUSE_GUI
