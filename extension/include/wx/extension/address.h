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
  
  /// Converts address to a real line number, filtering out markers
  /// and special characters.
  /// Returns 0 and bells on error in address, otherwise the vi line number,
  /// so subtract 1 for stc line number.
  int ToLine() const;
private:
  wxExEx* m_Ex;
  int m_Line;
};

class wxExSTC;

/// Support class for ex address range.
class WXDLLIMPEXP_BASE wxExAddressRange
{
public:
  /// Constructor.
  wxExAddressRange(wxExEx* ex);
  
  /// Constructor for a range from current position 
  /// extending with number of lines.
  wxExAddressRange(wxExEx* ex, int lines);
  
  /// Contructor for a range.
  /// - . : current line 
  /// - % : entire document
  /// - * : current screen
  /// <address>,<address> : range from begin and end address range.
  wxExAddressRange(wxExEx* ex, const wxString& range);
  
  /// Constructor with begin and end address range.
  wxExAddressRange(wxExEx* ex, const wxString& begin, const wxString& end);
  
  /// Deletes lines from range.
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
  bool Substitute(const wxString& command);
    
  /// Writes range to filename.
  bool Write(const wxString& filename) const;
  
  /// Yanks range.
  bool Yank() const;
private:  
  /// Gets substitute values out of command.
  bool Parse(const wxString& command, 
    wxString& pattern, wxString& replacement, wxString& options) const;
  /// Sets begin and end addresses.
  void Set(const wxString& begin, const wxString& end);
  /// Sets selection from begin to end address.
  /// Returns false if address cannot be related to a line number.
  bool SetSelection() const;

  wxString m_Replacement;
  
  wxExAddress m_Begin;
  wxExAddress m_End;
  wxExEx* m_Ex;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
#endif
