////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-input.h
// Purpose:   Declaration of wex::textctrl_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <string>
#include <wex/ex-command.h>

namespace wex
{
  class textctrl;

  /// Offers a class to relate textctrl to values with iterators.
  class textctrl_input
  {
  public:
    /// Constructor, fills values from config.
    /// The specified type determines which key to use
    /// to retrieve the values.
    textctrl_input(
      /// the ex_command type used to get config values
      ex_command::type_t type);

    /// Destructor, writes values to config.
    ~textctrl_input();

    /// Returns value on the list pointed to by iterator,
    /// or empty string, if iterator is at end.
    const std::string get() const;

    /// Sets first value on the list, and removes it
    /// at other positions if present.
    /// Sets iterator to begin of list.
    void set(const std::string& value);

    /// Sets first value on the list from specified text control.
    void set(const textctrl* tc);

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
      /// the text control
      textctrl* tc = nullptr);

    /// Sets all values (values might be empty).
    /// Sets iterator to begin of list.
    void set(const std::list<std::string>& values);

    /// Returns type.
    auto type() const { return m_type; }

    /// Gets all values.
    const auto& values() const { return m_values; }

  private:
    const ex_command::type_t               m_type;
    const std::string                      m_name;
    std::list<std::string>                 m_values;
    std::list<std::string>::const_iterator m_iterator;
  };
}; // namespace wex
