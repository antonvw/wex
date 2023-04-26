////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline-input.h
// Purpose:   Declaration of wex::ex_commandline_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/types.h>
#include <wex/factory/ex-command.h>

class wxTextEntryBase;

namespace wex
{
class ex_commandline;

/// Offers a class to relate ex_commandline to values with iterators.
class ex_commandline_input
{
public:
  /// Type for keeping the values.
  typedef wex::strings_t values_t;

  /// Constructor, fills values from config.
  /// The specified type determines which key to use
  /// to retrieve the values.
  ex_commandline_input(
    /// the ex_command type used to get config values
    ex_command::type_t type,
    /// the name, as used in config to store values
    const std::string& name = "ex-cmd.other");

  /// Destructor, writes values (with a max for integers on the list) to config.
  ~ex_commandline_input();

  /// Returns value on the list pointed to by iterator,
  /// or empty string, if iterator is at end.
  const std::string get() const;

  /// Sets first value on the list, and removes it
  /// at other positions if present.
  /// Sets iterator to begin of list.
  void set(const std::string& value);

  /// Sets first value on the list from specified commandline.
  void set(const ex_commandline* cl);

  /// Sets iterator according to specified key, and then
  /// sets value of text control (if not nullptr) to the list value
  /// related to iterator.
  /// Returns false if current list is empty, or key not ok.
  bool set(
    /// the key:
    /// - WXK_UP
    /// - WKK_DOWN
    /// - WXK_HOME
    /// - WXK_END
    /// - WXK_PAGEUP
    /// - WXK_PAGEDOWN
    int key,
    /// the text entry
    wxTextEntryBase* te = nullptr);

  /// Sets all values (values might be empty).
  /// Sets iterator to begin of list.
  void set(const values_t& values);

  /// Returns type.
  auto type() const { return m_type; }

  /// Returns the values.
  const auto& values() const { return m_values; }

private:
  const ex_command::type_t m_type;
  const std::string        m_name;

  values_t                 m_values;
  values_t::const_iterator m_iterator;
};
}; // namespace wex
