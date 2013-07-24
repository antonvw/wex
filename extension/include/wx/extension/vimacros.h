////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.h
// Purpose:   Declaration of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVIMACROS_H
#define _EXVIMACROS_H

#include <map>
#include <vector>
#include <wx/filename.h>
#include <wx/xml/xml.h>
#include <wx/extension/variable.h>

#if wxUSE_GUI

class wxExEx;

/// Offers the macro collection, and allows
/// recording and playback to vi (ex) component.
/// You can also use variables inside a macro (or in vi),
/// these are expanded while playing back.
class WXDLLIMPEXP_BASE wxExViMacros
{
public:  
  /// Default constructor.
  wxExViMacros();
  
  /// Returns all macro or variable names as an array of strings.
  const wxArrayString Get() const;
  
  /// Returns contents of macro or variable.
  const std::vector< wxString > Get(const wxString& macro) const;
  
  /// Returns number of macros and variables available.
  int GetCount() const;
  
  /// Returns current or last macro played back or variable expanded.
  const wxString& GetMacro() const {return m_Macro;};
  
  /// Returns register.
  const wxString GetRegister(const wxString& name) const;

  /// Returns all registers (macros with content) as an array of strings.
  /// The difference with Get() is, that this method 
  /// does not add variables, but adds the macro contents as well.
  const wxArrayString GetRegisters() const;
  
  /// Have macros been recorded or variables 
  /// expanded without calling SaveDocument.
  bool IsModified() const {return m_IsModified;};
  
  /// Is macro or variable recorded.
  bool IsRecorded(const wxString& macro) const;
  
  /// Is macro recorded.
  bool IsRecordedMacro(const wxString& macro) const;
  
  /// Are we playing back?
  bool IsPlayback() const {return m_IsPlayback;};
  
  /// Are we recording?
  bool IsRecording() const {return m_IsRecording;};
  
  /// Plays back macro a number of repeat times on the ex component.
  /// Returns true if all records could be executed.
  bool Playback(
    /// ex component to use
    wxExEx* ex, 
    /// macro name
    const wxString& macro, 
    /// number of times this maco is executed
    int repeat = 1);
  
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
  
  /// Sets register (overwrites existing macro).
  void SetRegister(const wxString& name, const wxString& value);
  
  /// Starts recording a macro (appends to 
  /// existing macro if macro is single upper case character).
  void StartRecording(const wxString& macro);
  
  /// Stops recording.
  void StopRecording();
  
  /// Expands variable to ex component.
  /// Returns true if variable could be expanded.
  static bool Expand(wxExEx* ex, const wxString& variable);
  
  /// Expands variable to value text.
  /// Returns true if variable could be expanded.
  static bool Expand(
    /// ex component to use
    wxExEx* ex, 
    /// variable name
    const wxString& variable, 
    /// value to receive contents
    wxString& value);
  
  /// Expands template variable.
  /// Returns true if the template file name exists,
  /// and all variables in it could be expanded.
  static bool ExpandTemplate(
    /// ex component to use
    wxExEx* ex, 
    /// variable (containing template file name)
    const wxExVariable& variable, 
    /// value to receive contents
    wxString& expanded);
  
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
  static void AskForInput();
  static bool Load(wxXmlDocument& doc);
  static const wxString Encode(const wxString& text);
  static const wxString Decode(const wxString& text);
    
  static bool m_IsExpand;
  static bool m_IsModified;
  static bool m_IsPlayback;
  static bool m_IsRecording;
  
  static wxString m_Macro;
  
  /// All macros, as a map of name and a vector of commands.
  static std::map <wxString, std::vector< wxString > > m_Macros;
  
  /// All variables, as a map of name and variable.
  static std::map<wxString, wxExVariable> m_Variables;
};
#endif // wxUSE_GUI
#endif
