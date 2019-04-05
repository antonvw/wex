////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
  class ex;
  class addressrange;

  /// Offers an address class to be used by vi address ranges.
  class address
  {
    friend addressrange;
  public:
    /// Constructor for an address.
    address(
      /// the ex (or vi) component
      ex* ex, 
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
    bool adjust_window(const std::string& text) const;
    
    /// Appends text to this address.
    bool append(const std::string& text) const;
    
    /// Returns false if flags are unsupported.
    bool flags_supported(const std::string& flags) const;
    
    /// Returns address as specified during construction.
    const auto & get() const {return m_Address;};
    
    /// If the line number was set using SetLine, it
    /// returns this line number, otherwise
    /// converts the address to a line number.
    /// This is the vi line number,
    /// so subtract 1 for stc line number.
    /// Returns 0 on error in address. 
    int get_line() const;
    
    /// Inserts text at this address.
    bool insert(const std::string& text) const;
    
    /// Marks this address.
    bool marker_add(char marker) const;
    
    /// Deletes marker (if this address concerns a marker).
    bool marker_delete() const;
    
    /// Append text from the specified register at this address, 
    /// default uses yank register.
    bool put(char name = '0') const;
    
    /// Read file at this address.
    bool read(const std::string& arg) const;
    
    /// Shows this address in the ex bar.
    bool write_line_number() const;
  private:
    /// Sets (vi) line number.
    void SetLine(int line);
    
    ex* m_Ex;
    int m_Line = 0;
    
    std::string m_Address; // set by address range
  };
};
