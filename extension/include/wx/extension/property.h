////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wxExProperty class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/log.h> 
#include <wx/stc/stc.h>
#include <wx/xml/xml.h>

/// This class defines our scintilla properties.
class WXDLLIMPEXP_BASE wxExProperty
{
public:
  /// Default constructor.
  wxExProperty(const wxXmlNode* node = nullptr) {
    if (node != nullptr) {
      m_Name = node->GetAttribute("name", "0");
      m_Value = node->GetNodeContent().Strip(wxString::both);
      if (!IsOk())
      {
        wxLogError("Illegal property name: %s or value: %s on line: %d",
          m_Name.c_str(),
          m_Value.c_str(),
          node->GetLineNumber());
      }}};
  
  /// Constructor using name, value pair.
  wxExProperty(const wxString& name, const wxString& value)
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
  const wxString& GetName() const {return m_Name;};

  /// Returns the value of this property.
  const wxString& GetValue() const {return m_Value;};

  /// Returns true if property is valid.
  bool IsOk() const {
    return !m_Name.empty() && !m_Value.empty();};
  
  /// Override this property (so does not apply this property).
  void Set(const wxString& value) {m_Value = value;};
private:
  wxString m_Name;
  wxString m_Value;
};
