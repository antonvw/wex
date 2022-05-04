////////////////////////////////////////////////////////////////////////////////
// Name:      macros.h
// Purpose:   Declaration of class wex::macros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

#include <wex/vi/macro-mode.h>
#include <wex/vi/variable.h>

#include <map>
#include <vector>

namespace wex
{
class ex;
class path;
class macro_fsm;

/// Offers the macro collection, and allows
/// recording and playback to vi (ex) component.
/// You can also use variables inside a macro (or in vi),
/// these are expanded while playing back.
/// \dot
/// digraph macro {
///   init -> idle       [style=dotted,label="start"];
///   idle -> recording  [label="q."];
///   recording -> idle  [label="q"];
///  }
/// \enddot
class macros
{
  friend class macro_fsm;

public:
  /// Type of macro key used for key_map.
  enum key_t
  {
    KEY_ALT,     ///< alt key
    KEY_CONTROL, ///< control key
    KEY_NORMAL,  ///< normal key (default)
  };

  /// Maps key to command.
  typedef std::map<int, std::string>                      keys_map_t;
  typedef std::map<std::string, std::string>              strings_map_t;
  typedef std::map<std::string, std::vector<std::string>> macros_map_t;
  typedef std::map<std::string, variable>                 variables_map_t;

  /// Default constructor.
  macros();

  /// Erases current macro from the vector and cleans it.
  /// Returns true if macro was erased.
  bool erase();

  /// Finds macro in macros or variables,
  /// and returns contents as a vector of strings,
  /// or empty vector if not found.
  const std::vector<std::string> find(const std::string& name) const;

  /// Returns all macro names as a vector of strings.
  /// Does not include registers.
  const std::vector<std::string> get() const;

  /// Returns abbreviations.
  const auto& get_abbreviations() const { return m_abbreviations; }

  /// Returns keys map.
  const keys_map_t& get_keys_map(key_t type = KEY_NORMAL) const;

  /// Returns commands for specified macro.
  const std::vector<std::string>
  get_macro_commands(const std::string& macro) const;

  /// Returns (string) map.
  const auto& get_map() const { return m_map; }

  /// Returns content of register.
  const std::string get_register(char name) const;

  /// Returns all registers (with content) as a vector of strings.
  /// Does not include macros.
  const std::vector<std::string> get_registers() const;

  /// Returns variables.
  const auto& get_variables() const { return m_variables; }

  /// Returns true if xml structure has been modified
  /// without being saved.
  bool is_modified() const { return m_is_modified; }

  /// Is macro or variable recorded.
  bool is_recorded(const std::string& macro) const;

  /// Is macro recorded.
  /// Does not check for variables.
  bool is_recorded_macro(const std::string& macro) const;

  /// Loads all macros (and variables) from xml document.
  /// Returns true if document is loaded (macros still can be empty).
  bool load_document();

  /// Returns the mode we are in.
  auto& mode() { return m_mode; }

  /// Returns the mode we are in.
  const auto& mode() const { return m_mode; }

  /// Returns the path with xml document.
  const wex::path path() const;

  /// Records text to current macro (or register) as a new command.
  /// The text to be recorded should be valid ex command,
  /// though it is not checked here.
  /// If you playback this macro the text
  /// is sent to the ex component to execute it, and then should be
  /// a valid command.
  ///  Returns false if text is not recorded.
  bool record(
    /// text to record
    const std::string& text,
    /// normally each record is a new command, if not,
    /// the text is appended after the last command
    bool new_command = true);

  /// Saves all macros (and variables) to xml document.
  /// If you specify only_if_modified, then document is only saved
  /// if it was modified (if macros have been recorded since last save).
  /// Returns true if document is saved.
  bool save_document(bool only_if_modified = true);

  /// Saves macro (and calls save_document).
  /// Returns false if macro does not exist.
  bool save_macro(const std::string& macro);

  /// Sets abbreviation (overwrites existing abbreviation).
  void set_abbreviation(const std::string& name, const std::string& value);

  /// Sets key map (overwrites existing map).
  void set_key_map(
    const std::string& name,
    const std::string& value,
    key_t              type = KEY_NORMAL);

  /// Sets map (overwrites existing map).
  void set_map(const std::string& name, const std::string& value);

  /// Sets register (overwrites existing register).
  /// The name should be a one letter register.
  /// Returns false if name is not appropriate.
  bool set_register(char name, const std::string& value);

  /// Returns number of macros and variables available.
  auto size() const { return m_macros.size() + m_variables.size(); }

  /// Does a recorded macro or variable starts with text.
  bool starts_with(const std::string_view& text);

private:
  template <typename S, typename T>
  void
  parse_node(const pugi::xml_node& node, const std::string& name, T& container);

  void parse_node_macro(const pugi::xml_node& node);
  void parse_node_variable(const pugi::xml_node& node);

  template <typename S, typename T>
  void set(
    T&                 container,
    const std::string& xpath,
    const std::string& name,
    const std::string& value);

  bool m_is_loaded{false}, m_is_modified{false};

  pugi::xml_document m_doc;

  strings_map_t m_abbreviations, m_map;

  /// Registers are 1 letter macros, and as such part of this container.
  macros_map_t m_macros;

  macro_mode m_mode;

  variables_map_t m_variables;

  keys_map_t m_map_alt_keys, m_map_control_keys, m_map_keys;
};
}; // namespace wex
