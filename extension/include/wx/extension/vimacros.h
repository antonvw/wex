////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.h
// Purpose:   Declaration of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>
#include <pugixml.hpp>
#include <wx/extension/path.h>
#include <wx/extension/variable.h>

#if wxUSE_GUI

class wxExEx;

enum wxExViMacrosKeyType
{
  KEY_ALT,       ///< alt key
  KEY_CONTROL,   ///< control key
  KEY_NORMAL,    ///< normal key (default)
};

/// Maps key to command.  
typedef std::map<int, std::string> wxExViMacrosMapType;

/// Offers the macro collection, and allows
/// recording and playback to vi (ex) component.
/// You can also use variables inside a macro (or in vi),
/// these are expanded while playing back.
class WXDLLIMPEXP_BASE wxExViMacros
{
public:  
  /// Returns all macro names as a vector of strings.
  /// Does not include registers.
  const std::vector< std::string > Get() const;
  
  /// Returns contents of macro as a vector of strings.
  const std::vector< std::string > Get(const std::string& name) const;
  
  /// Returns number of macros and variables available.
  auto GetCount() const {return m_Macros.size() + m_Variables.size();};
  
  /// Returns keys map.
  const auto & GetKeysMap(wxExViMacrosKeyType type = KEY_NORMAL) const {
    switch (type)
    {
      case KEY_ALT: return m_MapAltKeys;
      case KEY_CONTROL: return m_MapControlKeys;
      case KEY_NORMAL: return m_MapKeys;
    }};
  
  /// Returns current or last macro played back (or variable expanded).
  const auto& GetMacro() const {return m_Macro;};
  
  /// Returns (string) map.
  const auto & GetMap() const {return m_Map;};
  
  /// Returns content of register.
  const std::string GetRegister(const char name) const;

  /// Returns all registers (with content) as a vector of strings.
  /// Does not include macros.
  const std::vector< std::string > GetRegisters() const;
  
  /// Have macros been recorded (or variables 
  /// expanded) without calling SaveDocument.
  bool IsModified() const {return m_IsModified;};
  
  /// Is macro or variable recorded.
  bool IsRecorded(const std::string& macro) const;
  
  /// Is macro recorded.
  /// Does not check for variables.
  bool IsRecordedMacro(const std::string& macro) const;
  
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
    const std::string& macro, 
    /// number of times this maco is executed
    int repeat = 1);
  
  /// Records text to current macro (or register) as a new command.
  /// The text to be recorded should be valid ex command,
  /// though it is not checked here.
  /// If you playback this macro the text
  /// is sent to the ex component to execute it, and then should be
  /// a valid command.
  void Record(
    /// text to record
    const std::string& text, 
    /// normally each record is a new command, if not,
    /// the text is appended after the last command
    bool new_command = true);
  
  /// Sets abbreviation (overwrites existing abbreviation).
  void SetAbbreviation(const std::string& name, const std::string& value);
  
  /// Sets key map (overwrites existing map).
  void SetKeyMap(
    const std::string& name, 
    const std::string& value,
    wxExViMacrosKeyType type = KEY_NORMAL);
  
  /// Sets map (overwrites existing map).
  void SetMap(
    const std::string& name, 
    const std::string& value);

  /// Sets register (overwrites existing register).
  /// The name should be a one letter register.
  /// Returns false if name is not appropriate.
  bool SetRegister(const char name, const std::string& value);
  
  /// Starts recording a macro (appends to 
  /// existing macro if macro is single upper case character).
  void StartRecording(const std::string& macro);
  
  /// Does a recorded macro or variable starts with text.
  bool StartsWith(const std::string_view& text) const;
  
  /// Stops recording.
  void StopRecording();
  
  /// Expands variable to ex component.
  /// Returns true if variable could be expanded.
  static bool Expand(wxExEx* ex, const std::string& variable);
  
  /// Expands variable to value text.
  /// Returns true if variable could be expanded.
  static bool Expand(
    /// ex component to use
    wxExEx* ex, 
    /// variable name
    const std::string& variable, 
    /// value to receive contents
    std::string& value);
  
  /// Expands template variable.
  /// Returns true if the template file name exists,
  /// and all variables in it could be expanded.
  static bool ExpandTemplate(
    /// ex component to use
    wxExEx* ex, 
    /// variable (containing template file name)
    const wxExVariable& variable, 
    /// value to receive contents
    std::string& expanded);
  
  /// Returns the filename with xml document.
  static const wxExPath GetFileName();
  
  /// Returns abbreviations.
  static const auto & GetAbbreviations() {return m_Abbreviations;};
  
  /// Returns variables.
  static const auto& GetVariables() {return m_Variables;};
  
  /// Loads all macros (and variables) from xml document.
  /// Returns true if document is loaded (macros still can be empty).
  static bool LoadDocument();
  
  /// Saves all macros (and variables) to xml document.
  /// If you specify only_if_modified, then document is only saved
  /// if it was modified (if macros have been recorded since last save).
  /// Returns true if document is saved.
  static bool SaveDocument(bool only_if_modified = true);
private:  
  template <typename S, typename T> 
  void Set(
    T & container,
    const std::string& xpath,
    const std::string& name,
    const std::string& value);

  static void AskForInput();
  
  template <typename S, typename T> 
  static void ParseNode(
    const pugi::xml_node& node,
    const std::string& name,
    T & container);

  static void ParseNodeMacro(const pugi::xml_node& node);
  static void ParseNodeVariable(const pugi::xml_node& node);
  static void SaveMacro(const std::string& macro);
    
  static bool m_IsExpand;
  static bool m_IsModified;
  static bool m_IsPlayback;
  static bool m_IsRecording;
  
  static pugi::xml_document m_doc;
  
  static std::string m_Macro;
  
  /// All abbreviations, as a map of abbreviation and full text.
  static std::map<std::string, std::string> m_Abbreviations;
  
  /// All macros (and registers), as a map of name and a vector of commands.
  /// Registers are 1 letter macros.
  static std::map<std::string, std::vector<std::string> > m_Macros;

  /// String maps.
  static std::map<std::string, std::string> m_Map;

  /// Alt key maps.
  static wxExViMacrosMapType m_MapAltKeys;

  /// Control key maps.
  static wxExViMacrosMapType m_MapControlKeys;

  /// All normal key maps.
  static wxExViMacrosMapType m_MapKeys;

  /// All variables, as a map of name and variable.
  static std::map<std::string, wxExVariable> m_Variables;
};
#endif // wxUSE_GUI
