// Name:      vi-macros.h
// Purpose:   Declaration of class wex::vi_macros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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

  /// Offers the macro collection, and allows
  /// recording and playback to vi (ex) component.
  /// You can also use variables inside a macro (or in vi),
  /// these are expanded while playing back.
  class vi_macros
  {
    friend class vi_macros_fsm;
  public:  
    /// Type of macro key used for key_map.
    enum key_t
    {
      KEY_ALT,     ///< alt key
      KEY_CONTROL, ///< control key
      KEY_NORMAL,  ///< normal key (default)
    };
    
    /// static interface

    /// Maps key to command.  
    typedef std::map<int, std::string> keys_map_t;
    typedef std::map<std::string, std::string> strings_map_t;
    typedef std::map<std::string, std::vector<std::string> > macros_map_t;
    typedef std::map<std::string, variable> variables_map_t;

    /// Erases current macro from the vector and cleans it.
    /// Returns true if macro was erased.
    static bool erase();
    
    /// Returns all macro names as a vector of strings.
    /// Does not include registers.
    static const std::vector< std::string > get();
    
    /// Returns contents of macro as a vector of strings.
    static const std::vector< std::string > get(const std::string& name);
    
    /// Returns abbreviations.
    static const auto * get_abbreviations() {return m_abbreviations;};
    
    /// Returns the filename with xml document.
    static const path get_filename();
    
    /// Returns current or last macro played back (or variable expanded).
    static const auto & get_macro() {return m_macro;};
    
    /// Returns variables.
    static const auto * get_variables() {return m_variables;};
    
    /// Is macro or variable recorded.
    static bool is_recorded(const std::string& macro);
    
    /// Is macro recorded.
    /// Does not check for variables.
    static bool is_recorded_macro(const std::string& macro);
    
    /// Loads all macros (and variables) from xml document.
    /// Returns true if document is loaded (macros still can be empty).
    static bool load_document();
    
    /// Returns the mode we are in.  
    static auto & mode() {return m_mode;};

    /// Cleansup static data.
    /// Invoked once during app::on_exit.
    static void on_exit();
    
    /// Constructs static members.
    /// Invoked once during app::OnInit.
    static void on_init();
    
    /// Saves macro.
    static void save_macro(const std::string& macro);
        
    /// Saves all macros (and variables) to xml document.
    /// If you specify only_if_modified, then document is only saved
    /// if it was modified (if macros have been recorded since last save).
    /// Returns true if document is saved.
    static bool save_document(bool only_if_modified = true);

    /// Sets macro.
    static void set_macro(const std::string& macro) {m_macro = macro;};
    
    /// Does a recorded macro or variable starts with text.
    static bool starts_with(const std::string_view& text);

    /// other methods

    /// Default constructor.
    vi_macros();
    
    /// Returns keys map.
    const keys_map_t* get_keys_map(key_t type = KEY_NORMAL);
    
    /// Returns (string) map.
    const auto * get_map() const {return m_map;};
    
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
    auto size() const {return m_macros->size() + m_variables->size();};
  private:  
    template <typename S, typename T> 
    void set(
      T * container,
      const std::string& xpath,
      const std::string& name,
      const std::string& value);
    
    template <typename S, typename T> 
    static void parse_node(
      const pugi::xml_node& node,
      const std::string& name,
      T * container);

    static void parse_node_macro(const pugi::xml_node& node);
    static void parse_node_variable(const pugi::xml_node& node);

    static inline bool m_is_loaded {false}, m_is_modified {false};
    static inline std::string m_macro;
    
    static inline pugi::xml_document
      *m_doc {nullptr};

    static inline strings_map_t 
      *m_abbreviations {nullptr},
      *m_map {nullptr};

    /// Registers are 1 letter macros.
    static inline macros_map_t 
      *m_macros {nullptr};

    static inline vi_macros_mode
      *m_mode {nullptr};
    
    static inline variables_map_t 
      *m_variables {nullptr};

    static inline keys_map_t
      *m_map_alt_keys {nullptr}, 
      *m_map_control_keys {nullptr}, 
      *m_map_keys {nullptr};
  };
};
