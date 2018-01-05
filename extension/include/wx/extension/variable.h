////////////////////////////////////////////////////////////////////////////////
// Name:      variable.h
// Purpose:   Declaration of class wxExVariable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#if wxUSE_GUI

#include <pugixml.hpp>

class wxExEx;
class wxExSTCEntryDialog;

/// Offers variable support to be used in macros.
/// Variables are assigned from an xml node, and
/// you can expand them on an ex component.
class WXDLLIMPEXP_BASE wxExVariable
{
public:
  /// Default constructor.
  wxExVariable(const std::string& name = std::string())
    : m_Name(name) {;};
  
  /// Constructor that sets members using speecified xml node.
  wxExVariable(const pugi::xml_node& node);
  
  /// Expands variable to ex component.
  /// This might update the internal value.
  /// Returns true if variable could be expanded.
  bool Expand(wxExEx* ex);
  
  /// Expands variable to value text.
  /// Returns true if variable could be expanded.
  bool Expand(std::string& value, wxExEx* ex = nullptr) const;
  
  /// Returns variable name.
  const auto& GetName() const {return m_Name;};
  
  /// Returns variable value.
  const auto& GetValue() const {return m_Value;};
  
  /// Returns true if this variable is a built in.
  bool IsBuiltIn() const {return m_Type == VARIABLE_BUILTIN;};

  /// Returns true if this is an input template.  
  bool IsInput() const {return 
    m_Type == VARIABLE_INPUT || 
    m_Type == VARIABLE_INPUT_ONCE ||
    m_Type == VARIABLE_INPUT_SAVE;};

  /// Returns true if this variable is a template.
  bool IsTemplate() const {return m_Type == VARIABLE_TEMPLATE;};
  
  /// Save in xml node.
  void Save(pugi::xml_node& node, const std::string* value = nullptr);
  
  /// Sets the ask for input member, if appropriate for type.
  void SetAskForInput(bool value = true);
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
  bool ExpandBuiltIn(wxExEx* ex, std::string& expanded) const;
  bool ExpandInput(std::string& expanded) const;
  
  bool m_AskForInput{true};
    
  // no const members because of assignment in wxExViMacrosFSM
  int m_Type{VARIABLE_INPUT_SAVE};
  std::string m_Name;
  std::string m_Prefix;
  std::string m_Value;
  
  // The dialog used.
  static wxExSTCEntryDialog* m_Dialog;
};
#endif // wxUSE_GUI
