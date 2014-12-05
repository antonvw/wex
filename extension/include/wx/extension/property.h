////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wxExProperty class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXPROPERTY_H
#define _EXPROPERTY_H

class wxXmlNode;
class wxStyledTextCtrl;

/// This class defines our scintilla properties.
class WXDLLIMPEXP_BASE wxExProperty
{
public:
  /// Default constructor.
  wxExProperty(const wxXmlNode* node = NULL) {
    if (node != NULL) Set(node);};
  
  /// Constructor using name, value pair.
  wxExProperty(const wxString& name, const wxString& value)
    : m_Name(name)
    , m_Value(value){;};

  /// Applies this property to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Resets this property (resets the value of this property
  /// on the stc component, but does not change the value).
  void ApplyReset(wxStyledTextCtrl* stc) const;
  
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
  void Set(const wxXmlNode* node);
  
  wxString m_Name;
  wxString m_Value;
};
#endif
