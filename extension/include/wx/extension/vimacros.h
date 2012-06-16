////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.h
// Purpose:   Declaration of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVIMACROS_H
#define _EXVIMACROS_H

#include <map>
#include <vector>
#include <wx/filename.h>
#include <wx/xml/xml.h>

#if wxUSE_GUI

class wxExEx;

/// Offers variable support to be used in macros.
class WXDLLIMPEXP_BASE wxExVariable
{
public:
  /// Default constructor.
  wxExVariable();
  
  /// Constructor.
  wxExVariable(const wxXmlNode* node);
  
  /// Clears value, depending on type.
  void Clear();
  
  /// Expands variable to ex component.
  /// Returns true if variable could be expanded.
  bool Expand(bool playback, wxExEx* ex);
  
  /// Gets name.
  const wxString& GetName() const {return m_Name;};
  
  /// Gets prefix.
  const wxString& GetPrefix() const {return m_Prefix;};
  
  /// Gets value.
  const wxString& GetValue() const {return m_Value;};
  
  /// Save in xml node.
  void Save(wxXmlNode* node);
private:  
  bool ExpandBuiltIn(wxExEx* ex, wxString& expanded) const;
  bool ExpandInput(bool playback, wxString& expanded);
    
  int m_Type;
  wxString m_Name;
  wxString m_Prefix;
  
  /// We keep values of input variables,
  /// so, while playing back, you have to enter them only once.
  /// The values are cleared each time you start playback.
  wxString m_Value;
};

/// Offers the macro collection, and allows
/// recording and playback to vi (ex) component.
/// You can also use variables inside a macro (or in vi),
/// these are expanded while playing back.
class WXDLLIMPEXP_BASE wxExViMacros
{
public:  
  /// Default constructor.
  wxExViMacros();
  
  /// Expands variable to ex component.
  /// Returns true if variable could be expanded.
  bool Expand(wxExEx* ex, const wxString& variable);
  
  /// Returns all macros (names) as an array of strings.
  const wxArrayString Get() const;
  
  /// Returns contents of macro (for testing).
  const std::vector< wxString > Get(const wxString& macro) const;
  
  /// Returns current or last macro.
  const wxString& GetMacro() {return m_Macro;};
  
  /// Have macros been recorded without calling SaveDocument.
  bool IsModified() {return m_IsModified;};
  
  /// Is macro recorded.
  bool IsRecorded(const wxString& macro = wxEmptyString) const;
  
  /// Are we playing back?
  bool IsPlayback() const {return m_IsPlayback;};
  
  /// Are we recording?
  bool IsRecording() const {return m_IsRecording;};
  
  /// Plays back macro a number of repeat times on the ex component.
  /// Returns true if all records could be executed
  /// (or if repeat is 0).
  bool Playback(wxExEx* ex, const wxString& macro, int repeat = 1);
  
  /// Records text to current macro as a new command.
  /// The text to be recorded should be valid ex command,
  /// though it is not checked here.
  /// If you playback this macro the text
  /// is sent to the ex component to execute it, and then should be
  /// a valid command.
  void Record(
    /// text to record
    const wxString& text, 
    /// normally each record is a new command, if not,
    /// the text is appended after the last command
    bool new_command = true);
  
  /// Starts recording a macro (overwrites 
  /// existing macro if macro starts with lower case).
  void StartRecording(const wxString& macro);
  
  /// Stops recording.
  void StopRecording();
  
  /// Returns the filename with xml document.
  static const wxFileName GetFileName();
  
  /// Loads all macros (and variables) from xml document.
  /// Returns true if document is loaded (macros still can be empty).
  static bool LoadDocument();
  
  /// Saves all macros (and variables) to xml document.
  /// If you specify only_if_modified, then document is only saved
  /// if it was modified (if macros have been recorded since last save).
  /// Returns true if document is saved.
  static bool SaveDocument(bool only_if_modified = true);
private:  
  static bool Load(wxXmlDocument& doc);
  static const wxString Encode(const wxString& text, bool& encoded);
  static const wxString Decode(const wxString& text);
    
  static bool m_IsModified;
  
  /// All macros, as a map of name and a vector of commands.
  static std::map <wxString, std::vector< wxString > > m_Macros;
  
  /// All variables, as a map of name and variable.
  static std::map<wxString, wxExVariable> m_Variables;
  
  bool m_IsPlayback;
  bool m_IsRecording;
  
  wxString m_Macro;
};
#endif // wxUSE_GUI
#endif
