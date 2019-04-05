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
    enum key_t
    {
      KEY_ALT,     ///< alt key
      KEY_CONTROL, ///< control key
      KEY_NORMAL,  ///< normal key (default)
    };

    /// Default constructor.
    vi_macros();
    
    /// Returns keys map.
    const auto & get_keys_map(key_t type = KEY_NORMAL) const {
      switch (type)
      {
        case KEY_ALT: return m_MapAltKeys;
        case KEY_CONTROL: return m_MapControlKeys;
        default: return m_MapKeys;
      }};
    
    /// Returns (string) map.
    const auto & get_map() const {return m_Map;};
    
    /// Returns content of register.
    const std::string get_register(const char name) const;

    /// Have macros been recorded (or variables 
    /// expanded) without calling save_document.
    bool is_modified() const {return m_is_modified;};
    
    /// Records text to current macro (or register) as a new command.
    /// The text to be recorded should be valid ex command,
    /// though it is not checked here.
    /// If you playback this macro the text
    /// is sent to the ex component to execute it, and then should be
    /// a valid command.
    void record(
      /// text to record
      const std::string& text, 
      /// normally each record is a new command, if not,
      /// the text is appended after the last command
      bool new_command = true);
    
    /// Returns all registers (with content) as a vector of strings.
    /// Does not include macros.
    const std::vector< std::string > registers() const;
    
    /// Sets abbreviation (overwrites existing abbreviation).
    void set_abbreviation(const std::string& name, const std::string& value);
    
    /// Sets key map (overwrites existing map).
    void set_key_map(
      const std::string& name, 
      const std::string& value,
      key_t type = KEY_NORMAL);
    
    /// Sets map (overwrites existing map).
    void set_map(
      const std::string& name, 
      const std::string& value);

    /// Sets register (overwrites existing register).
    /// The name should be a one letter register.
    /// Returns false if name is not appropriate.
    bool set_register(const char name, const std::string& value);
    
    /// Returns number of macros and variables available.
    auto size() const {return m_Macros.size() + m_Variables.size();};
    
    /// Returns all macro names as a vector of strings.
    /// Does not include registers.
    static const std::vector< std::string > get();
    
    /// Returns contents of macro as a vector of strings.
    static const std::vector< std::string > get(const std::string& name);
    
    /// Returns abbreviations.
    static const auto & get_abbreviations() {return m_Abbreviations;};
    
    /// Returns the filename with xml document.
    static const path get_filename();
    
    /// Returns current or last macro played back (or variable expanded).
    static const auto& get_macro() {return m_Macro;};
    
    /// Returns variables.
    static const auto& get_variables() {return m_Variables;};
    
    /// Is macro or variable recorded.
    static bool is_recorded(const std::string& macro);
    
    /// Is macro recorded.
    /// Does not check for variables.
    static bool is_recorded_macro(const std::string& macro);
    
    /// Loads all macros (and variables) from xml document.
    /// Returns true if document is loaded (macros still can be empty).
    static bool load_document();
    
    /// Returns the mode we are in.  
    static auto mode() {return m_Mode;};

    /// Saves all macros (and variables) to xml document.
    /// If you specify only_if_modified, then document is only saved
    /// if it was modified (if macros have been recorded since last save).
    /// Returns true if document is saved.
    static bool save_document(bool only_if_modified = true);

    /// Does a recorded macro or variable starts with text.
    static bool starts_with(const std::string_view& text);
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

    static void save_macro(const std::string& macro);
    static void ParseNodeMacro(const pugi::xml_node& node);
    static void ParseNodeVariable(const pugi::xml_node& node);

    static vi_macros_mode* m_Mode;
    static bool m_is_modified;
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
