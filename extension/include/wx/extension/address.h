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
class wxExIndicator;
class wxExSTC;

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
  
  /// If the line number was set using SetLine, it
  /// returns this line number, otherwise
  /// converts the address to a line number.
  /// This is the vi line number,
  /// so subtract 1 for stc line number.
  /// Returns 0 on error in address. 
  int GetLine() const;
  
  /// Deletes marker (if this address concerns a marker).
  void MarkerDelete() const;
  
  /// Sets (vi) line number.
  void SetLine(int line);
private:
  wxExEx* m_Ex;
  int m_Line;
};

/// Offers an address range for vi (ex).
/// - If a range is already selected on the STC component, then
///   that range is used.
/// - Otherwise the range is derived from line numbers, 
///   and all methods return false if this is not possible.
class WXDLLIMPEXP_BASE wxExAddressRange
{
public:
  /// Constructor for a range from current position 
  /// extending with number of lines.
  wxExAddressRange(
    /// the ex (or vi) component
    wxExEx* ex, 
    /// lines 1 is current line only
    /// lines 0 is illegal
    int lines = 1);
  
  /// Contructor for a range.
  wxExAddressRange(
    /// the ex (or vi) component
    wxExEx* ex, 
    /// the range, being a string containing:
    /// - . : current line 
    /// - % : entire document
    /// - * : current screen visible area
    /// - x,y: range from begin x and end y address.
    const wxString& range);
  
  /// Deletes range.
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
  
  /// Is this range is ok (or a selection is active).
  bool IsOk() const;
  
  /// Moves range to destination.
  bool Move(const wxExAddress& destination) const;
  
  /// Substitutes range by /pattern/replacement/options in command.
  /// Pattern might contain:
  /// - $ to match a line end
  /// Replacement might contain:
  /// - & or \0 to represent the target in the replacement
  /// - \U to convert target to uppercase 
  /// - \L to convert target to lowercase
  /// - ~ to use previous replacement
  /// Options can be:
  /// - c : Ask for confirm
  /// - i : Case insensitive
  /// - g : Do global on line, without this flag replace first match only
  /// e.g. /$/EOL appends the string EOL at the end of each line. 
  /// Merging is not yet possible using a \n target,
  /// you can create a macro for that.  
  bool Substitute(const wxString& command);
    
  /// Writes range to filename.
  bool Write(const wxString& filename) const;
  
  /// Yanks range.
  bool Yank() const;
private:  
  const wxString BuildReplacement(const wxString& text) const;
  int Confirm(
    const wxString& pattern, 
    const wxString& replacement, 
    const wxExIndicator& ind);
  bool Parse(const wxString& command, 
    wxString& pattern, wxString& replacement, wxString& options) const;
  void Set(const wxString& begin, const wxString& end) {
    m_Begin.assign(begin);
    m_End.assign(end);};
  void Set(int begin, int end) {
    m_Begin.SetLine(begin);
    m_End.SetLine(end);};
  void Set(wxExAddress& begin, wxExAddress& end, int lines);
  bool SetSelection() const;

  static wxString m_Replacement;
  
  wxExAddress m_Begin;
  wxExAddress m_End;
  wxExEx* m_Ex;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
