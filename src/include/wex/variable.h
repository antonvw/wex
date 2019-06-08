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
  class stc_entry_dialog;

  /// Offers variable support to be used in macros.
  /// Variables are assigned from an xml node, and
  /// you can expand them on an ex component.
  class variable
  {
  public:
    /// Default constructor.
    variable(const std::string& name = std::string())
      : m_Name(name) {;};
    
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
    const auto& get_name() const {return m_Name;};
    
    /// Returns variable value.
    const auto& get_value() const {return m_Value;};
    
    /// Returns true if this variable is a built in.
    bool is_builtin() const {return m_Type == VARIABLE_BUILTIN;};

    /// Returns true if this is an input template.  
    bool is_input() const {return 
      m_Type == VARIABLE_INPUT || 
      m_Type == VARIABLE_INPUT_ONCE ||
      m_Type == VARIABLE_INPUT_SAVE;};

    /// Returns true if this variable is a template.
    bool is_template() const {return m_Type == VARIABLE_TEMPLATE;};
    
    /// Save in xml node.
    void save(pugi::xml_node& node, const std::string* value = nullptr);
    
    /// Sets the ask for input member, if appropriate for type.
    void set_ask_for_input(bool value = true);
  private:  
    // Several types of variables are supported.
    // See xml file.
    enum
    {
      VARIABLE_BUILTIN,     // a builtin variable like "Created"
      VARIABLE_ENVIRONMENT, // an environment variable like ENV
      VARIABLE_INPUT,       // input from user
      VARIABLE_INPUT_ONCE,  // input once from user, save value in xml file
      VARIABLE_INPUT_SAVE,  // input from user, save value in xml file
      VARIABLE_READ,        // read value from macros xml file
      VARIABLE_TEMPLATE     // read value from a template file
    };

    bool CheckLink(std::string& value) const;
    bool ExpandBuiltIn(ex* ex, std::string& expanded) const;
    bool ExpandInput(std::string& expanded) const;
    
    bool m_AskForInput{true};
      
    // no const members because of assignment in vi_macros_fsm
    int m_Type{VARIABLE_INPUT_SAVE};
    std::string m_Name;
    std::string m_Prefix;
    std::string m_Value;
    
    // The dialog used.
    static stc_entry_dialog* m_Dialog;
  };
};
