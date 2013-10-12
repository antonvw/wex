////////////////////////////////////////////////////////////////////////////////
// Name:      address.h
// Purpose:   Declaration of class wxExAddress and wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXADDRESS_H
#define _EXADDRESS_H

#if wxUSE_GUI

class wxExEx;
class wxExAddressRange;

/// Support class for ex address.
class WXDLLIMPEXP_BASE wxExAddress : public wxString
{
  friend wxExAddressRange;
public:
  /// Constructor.
  wxExAddress(wxExEx* ex, const wxString& address = wxEmptyString);
  
  /// Gets substitute values out of address.
  bool Substitute(wxString& pattern, wxString& replacement, wxString& options);
  
  /// Converts address to a real line number, filtering out markers
  /// and special characters.
  /// Returns 0 and bells on error in address, otherwise the vi line number,
  /// so subtract 1 for stc line number.
  int ToLine() const;
private:
  wxExEx* m_Ex;
  int m_Line;
  int m_Pos;
};

class wxExSTC;

/// Support class for ex address range.
class WXDLLIMPEXP_BASE wxExAddressRange
{
public:
  /// Constructor.
  /// If there is a selection, sets begin and end position
  /// accordingly.
  wxExAddressRange(wxExEx* ex);
  
  /// Constructor for a range from current position 
  // extending with number of lines.
  wxExAddressRange(wxExEx* ex, int lines);
  
  /// Constructor with begin and end address range.
  wxExAddressRange(wxExEx* ex, const wxString& begin, const wxString& end);
  
  /// Deletes lines from range.
  /// Returns false if address cannot be related to a line number.
  bool Delete() const;
  
  /// Filters range with command.
  /// The address range is used as input for the command,
  /// and the output of the command replaces the address range.
  bool Filter(const wxString& command) const;
  
  /// Gets begin address.
  const wxExAddress& GetBegin() const {return m_Begin;};
  
  /// Gets end address.
  const wxExAddress& GetEnd() const {return m_End;};
  
  /// Indents range.
  bool Indent(bool forward = true) const;
  
  /// Is this range ok.
  bool IsOk() const;
  
  /// Moves range to destination.
  bool Move(const wxExAddress& destination) const;
  
  /// Sets begin and end addresses to the same value.
  void Set(const wxString& value);
  
  /// Sets begin and end addresses.
  void Set(const wxString& begin, const wxString& end);
    
  /// Writes range to filename.
  bool Write(const wxString& filename) const;
  
  /// Yanks range.
  bool Yank() const;
private:  
  /// Sets selection from begin to end address.
  /// Returns false if address cannot be related to a line number.
  bool SetSelection() const;
  
  wxExAddress m_Begin;
  wxExAddress m_End;
  wxExEx* m_Ex;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
