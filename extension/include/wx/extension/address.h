////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wxExAddress and wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXADDRESS_H
#define _EXADDRESS_H

#if wxUSE_GUI

class wxExEx;
class wxExAddressRange;

/// Offers an address class to be used by vi address ranges.
class WXDLLIMPEXP_BASE wxExAddress : public wxString
{
  friend class wxExAddressRange;
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
    const wxString& address);
  
  /// Converts the address to a line number.
  /// Returns 0 and bells on error in address, 
  /// otherwise returns the vi line number,
  /// so subtract 1 for stc line number.
  int ToLine() const;
private:
  wxExEx* m_Ex;
  int m_Line;
};

class wxExSTC;

/// Offers an address range for vi (ex).
class WXDLLIMPEXP_BASE wxExAddressRange
{
public:
  /// Constructor for a range from current position 
  /// extending with number of lines (1 is current line).
  wxExAddressRange(wxExEx* ex, int lines = 1);
  
  /// Contructor for a range.
  wxExAddressRange(
    /// the ex (or vi) component
    wxExEx* ex, 
    /// the range, being a string containing:
    /// - . : current line 
    /// - % : entire document
    /// - * : current screen visible area
    /// - x, y: range from begin x and end y address.
    const wxString& range);
  
  /// Deletes range.
  /// Returns false if address cannot be related to a line number.
  bool Delete(bool show_message = true) const;
  
  /// Filters range with command.
  /// The address range is used as input for the command,
  /// and the output of the command replaces the address range.
  /// For example, the command:
  /// :96,99!sort
  /// will pass lines 96 through 99 through the sort filter and 
  /// replace those lines with the output of sort.  
  bool Filter(const wxString& command) const;
  
  /// Indents range.
  bool Indent(bool forward = true) const;
  
  /// Is this range ok.
  bool IsOk() const;
  
  /// Moves range to destination.
  bool Move(const wxExAddress& destination) const;
  
  /// Substitutes range by /pattern/replace/options in command.
  /// Options can be:
  /// - c : Ask for confirm
  /// - i : Case insensitive
  /// - g : Do global on line, without this flag replace first match only
  bool Substitute(const wxString& command);
    
  /// Writes range to filename.
  bool Write(const wxString& filename) const;
  
  /// Yanks range.
  bool Yank() const;
private:  
  /// Gets substitute values from command.
  bool Parse(const wxString& command, 
    wxString& pattern, wxString& replacement, wxString& options) const;
  /// Sets begin and end addresses.
  void Set(const wxString& begin, const wxString& end);
  /// Sets selection from begin to end address.
  /// Returns false if address cannot be related to a line number.
  bool SetSelection(bool line_end_pos = false) const;
  bool SetSelection(
    int begin_line, int end_line, bool line_end_pos = false) const;

  wxString m_Replacement;
  
  wxExAddress m_Begin;
  wxExAddress m_End;
  wxExEx* m_Ex;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
