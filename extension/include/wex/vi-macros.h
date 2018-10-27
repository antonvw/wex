// Name:      vi-macros.h
// Purpose:   Declaration of class wex::vi_macros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>
#include <pugixml.hpp>
#include <wex/variable.h>

namespace wex
{
  class ex;
  class path;
  class vi_macros_fsm;
  class vi_macros_mode;

  /// Maps key to command.  
  typedef std::map<int, std::string> vi_macros_maptype;

  /// Offers the macro collection, and allows
  /// recording and playback to vi (ex) component.
  /// You can also use variables inside a macro (or in vi),
  /// these are expanded while playing back.
  class vi_macros
  {
    friend class vi_macros_fsm;
  public:  
    enum key_type
    {
      KEY_ALT,     ///< alt key
      KEY_CONTROL, ///< control key
      KEY_NORMAL,  ///< normal key (default)
    };

    /// Default constructor.
    vi_macros();
    
    /// Returns number of macros and variables available.
    auto GetCount() const {return m_Macros.size() + m_Variables.size();};
    
    /// Returns keys map.
    const auto & GetKeysMap(key_type type = KEY_NORMAL) const {
      switch (type)
      {
        case KEY_ALT: return m_MapAltKeys;
        case KEY_CONTROL: return m_MapControlKeys;
        default: return m_MapKeys;
      }};
    
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
      key_type type = KEY_NORMAL);
    
    /// Sets map (overwrites existing map).
    void SetMap(
      const std::string& name, 
      const std::string& value);

    /// Sets register (overwrites existing register).
    /// The name should be a one letter register.
    /// Returns false if name is not appropriate.
    bool SetRegister(const char name, const std::string& value);
    
    /// Returns all macro names as a vector of strings.
    /// Does not include registers.
    static const std::vector< std::string > Get();
    
    /// Returns contents of macro as a vector of strings.
    static const std::vector< std::string > Get(const std::string& name);
    
    /// Returns the filename with xml document.
    static const path GetFileName();
    
    /// Returns abbreviations.
    static const auto & GetAbbreviations() {return m_Abbreviations;};
    
    /// Returns current or last macro played back (or variable expanded).
    static const auto& GetMacro() {return m_Macro;};
    
    /// Returns variables.
    static const auto& GetVariables() {return m_Variables;};
    
    /// Is macro or variable recorded.
    static bool IsRecorded(const std::string& macro);
    
    /// Is macro recorded.
    /// Does not check for variables.
    static bool IsRecordedMacro(const std::string& macro);
    
    /// Loads all macros (and variables) from xml document.
    /// Returns true if document is loaded (macros still can be empty).
    static bool LoadDocument();
    
    /// Returns the mode we are in.  
    static auto Mode() {return m_Mode;};

    /// Saves all macros (and variables) to xml document.
    /// If you specify only_if_modified, then document is only saved
    /// if it was modified (if macros have been recorded since last save).
    /// Returns true if document is saved.
    static bool SaveDocument(bool only_if_modified = true);

    /// Does a recorded macro or variable starts with text.
    static bool StartsWith(const std::string_view& text);
  private:  
    template <typename S, typename T> 
    void Set(
      T & container,
      const std::string& xpath,
      const std::string& name,
      const std::string& value);
    
    template <typename S, typename T> 
    static void ParseNode(
      const pugi::xml_node& node,
      const std::string& name,
      T & container);

    static void ParseNodeMacro(const pugi::xml_node& node);
    static void ParseNodeVariable(const pugi::xml_node& node);
    static void SaveMacro(const std::string& macro);

    static vi_macros_mode* m_Mode;
    static bool m_IsModified;
    static pugi::xml_document m_doc;
    static std::string m_Macro;
    
    /// All abbreviations, as a map of abbreviation and full text.
    static std::map<std::string, std::string> m_Abbreviations;
    /// String maps.
    static std::map<std::string, std::string> m_Map;
    /// All macros (and registers), as a map of name and a vector of commands.
    /// Registers are 1 letter macros.
    static std::map<std::string, std::vector<std::string> > m_Macros;
    /// All variables, as a map of name and variable.
    static std::map<std::string, variable> m_Variables;

    static vi_macros_maptype 
      m_MapAltKeys, m_MapControlKeys, m_MapKeys; /// All normal key maps.
  };
};
