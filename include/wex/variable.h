////////////////////////////////////////////////////////////////////////////////
// Name:      variable.h
// Purpose:   Declaration of class wex::variable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp>

namespace wex
{
  class ex;

  /// Offers variable support to be used in macros.
  /// Variables are assigned from an xml node, and
  /// you can expand them on an ex component.
  class variable
  {
  public:
    /// Sets argument (for a PROCESS variable).
    static void set_argument(const std::string& val);

    /// Default constructor.
    variable(const std::string& name = std::string());

    /// Constructor that sets members using specified xml node.
    variable(const pugi::xml_node& node);

    /// Expands variable to ex component.
    /// This might update the internal value.
    /// Returns true if variable could be expanded.
    bool expand(ex* ex);

    /// Expands variable to value text.
    /// Returns true if variable could be expanded.
    bool expand(std::string& value, ex* ex = nullptr) const;

    /// Returns variable name.
    const auto& get_name() const { return m_name; };

    /// Returns variable value.
    const auto& get_value() const { return m_value; };

    /// Returns true if this variable is a built in.
    bool is_builtin() const;

    /// Returns true if this is an input template.
    bool is_input() const;

    /// Returns true if this variable is a template.
    bool is_template() const;

    /// Save in xml node.
    void save(pugi::xml_node& node, const std::string* value = nullptr);

    /// Sets the ask for input member, if appropriate for type.
    void set_ask_for_input(bool value = true);

  private:
    enum class input_t;

    bool check_link(std::string& value) const;
    bool expand_builtin(ex* ex, std::string& expanded) const;
    bool expand_input(std::string& expanded) const;

    bool m_ask_for_input{true};

    // no const members because of assignment in macro_fsm
    input_t m_type;

    std::string m_format, m_name, m_prefix, m_value;

    static inline std::string m_argument;
  };
}; // namespace wex
