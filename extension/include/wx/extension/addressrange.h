////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.h
// Purpose:   Declaration of class wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/address.h>
#include <wx/extension/indicator.h>

#if wxUSE_GUI

class wxExProcess;
class wxExSTC;

/// Offers an address range for vi (ex).
/// - The range is derived from a number of lines, 
/// - or by a range string (including visual range for 
///   already selected text on the STC component).
/// All methods return false if the range is not ok.
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
  
  /// Contructor for a range (including visual range).
  wxExAddressRange(
    /// the ex (or vi) component
    wxExEx* ex, 
    /// the range, being a string containing:
    /// - . : current line 
    /// - % : entire document
    /// - * : current screen visible area
    /// - x,y: range from begin x and end y address.
    const wxString& range);

  /// Destructor.
 ~wxExAddressRange();
  
  /// Changes range.
  bool Change(const wxString& command) const;
  
  /// Copies range to destination.
  bool Copy(const wxExAddress& destination) const;
  
  /// Deletes range.
  bool Delete(bool show_message = true) const;
  
  /// Filters range with command.
  /// The address range is used as input for the command,
  /// and the output of the command replaces the address range.
  /// For example: wxAddressRange(96, 99).Filter("sort")
  /// or (ex command::96,99!sort)
  /// will pass lines 96 through 99 through the sort filter and 
  /// replace those lines with the output of sort.  
  /// Of course, you could also do: wxAddressRange(96,99).Sort().
  bool Filter(const wxString& command);
  
  /// Performs the global command on this range.
  bool Global(const wxString& command) const;
  
  /// Indents range.
  bool Indent(bool forward = true) const;
  
  /// Is this range ok.
  bool IsOk() const;
  
  /// Joins range.
  bool Join() const;
  
  /// Moves range to destination.
  bool Move(const wxExAddress& destination) const;
  
  /// Prints range to print file.
  bool Print(const wxString& flags = wxEmptyString) const;
  
  /// Sorts range, with optional parameters:
  /// -u to sort unique lines
  /// -r to sort reversed (descending)
  ///  - x,y sorts rectangle within range: x start col, y end col (exclusive).
  bool Sort(const wxString& parameters = wxEmptyString) const;
  
  /// Substitutes range.
  bool Substitute(
    /// text format: /pattern/replacement/options
    /// Pattern might contain:
    /// - $ to match a line end
    /// Replacement might contain:
    /// - & or \0 to represent the target in the replacement
    /// - \U to convert target to uppercase 
    /// - \L to convert target to lowercase
    /// Options can be:
    /// - c : Ask for confirm
    /// - i : Case insensitive
    /// - g : Do global on line, without this flag replace first match only
    /// e.g. /$/EOL appends the string EOL at the end of each line. 
    /// Merging is not yet possible using a \n target,
    /// you can create a macro for that.  
    const wxString& text,
    /// cmd is one of s, & or ~
    /// - s : default, normal substitute
    /// - & : repeat last substitute (text contains options)
    /// - ~ : repeat last substitute with pattern from find replace data (text contains options)
    const char cmd = 's');
    
  /// Writes range to filename.
  bool Write(const wxString& filename) const;
  
  /// Yanks range to register, default to yank register.
  bool Yank(const char name = '0') const;
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

  static wxString m_Pattern;
  static wxString m_Replacement;
  
  const wxExIndicator m_FindIndicator;
  wxExAddress m_Begin;
  wxExAddress m_End;
  wxExEx* m_Ex;
  wxExProcess* m_Process;
  wxExSTC* m_STC;
};
#endif // wxUSE_GUI
