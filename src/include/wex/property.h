////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wex::property class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp> 
#include <wx/stc/stc.h>
#include <wex/log.h>

namespace wex
{
  /// This class defines our scintilla properties.
  class property
  {
  public:
    /// Default constructor.
    property(const pugi::xml_node& node = pugi::xml_node()) {
      if (!node.empty()) {
        m_name = node.attribute("name").value();
        m_value = node.text().get();
        if (m_value.empty())
        {
          log("empty property") << m_name << node;
        }}};
    
    /// Constructor using name, value pair.
    property(const std::string& name, const std::string& value)
      : m_name(name)
      , m_value(value){;};

    /// Applies this property to stc component.
    void apply(wxStyledTextCtrl* stc) const {
      if (is_ok()) stc->SetProperty(m_name, m_value);};

    /// Resets this property (resets the value of this property
    /// on the stc component, but does not change the value).
    void apply_reset(wxStyledTextCtrl* stc) const {
      stc->SetProperty(m_name, wxEmptyString);};
    
    /// Returns true if property is valid.
    bool is_ok() const {
      return !m_name.empty() && !m_value.empty();};
    
    /// Returns the name of this property.
    const auto & name() const {return m_name;};

    /// Override this property (so does not apply this property).
    void set(const std::string& value) {m_value = value;};

    /// Returns the value of this property.
    const auto & value() const {return m_value;};
  private:
    std::string m_name, m_value;
  };
};
