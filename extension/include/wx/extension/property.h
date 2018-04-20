////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wxExProperty class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <pugixml.hpp> 
#include <wx/stc/stc.h>
#include <wx/extension/log.h>

/// This class defines our scintilla properties.
class WXDLLIMPEXP_BASE wxExProperty
{
public:
  /// Default constructor.
  wxExProperty(const pugi::xml_node& node = pugi::xml_node()) {
    if (!node.empty()) {
      m_Name = node.attribute("name").value();
      m_Value = node.text().get();
      if (m_Value.empty())
      {
        wxExLog("empty property") << m_Name << node;
      }}};
  
  /// Constructor using name, value pair.
  wxExProperty(const std::string& name, const std::string& value)
    : m_Name(name)
    , m_Value(value){;};

  /// Applies this property to stc component.
  void Apply(wxStyledTextCtrl* stc) const {
    if (IsOk()) stc->SetProperty(m_Name, m_Value);};

  /// Resets this property (resets the value of this property
  /// on the stc component, but does not change the value).
  void ApplyReset(wxStyledTextCtrl* stc) const {
    stc->SetProperty(m_Name, wxEmptyString);};
  
  /// Returns the name of this property.
  const auto & GetName() const {return m_Name;};

  /// Returns the value of this property.
  const auto & GetValue() const {return m_Value;};

  /// Returns true if property is valid.
  bool IsOk() const {
    return !m_Name.empty() && !m_Value.empty();};
  
  /// Override this property (so does not apply this property).
  void Set(const std::string& value) {m_Value = value;};
private:
  std::string m_Name;
  std::string m_Value;
};
