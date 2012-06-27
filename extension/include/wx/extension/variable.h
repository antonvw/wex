////////////////////////////////////////////////////////////////////////////////
// Name:      variable.h
// Purpose:   Declaration of class wxExVariable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVARIABLE_H
#define _EXVARIABLE_H

#include <wx/xml/xml.h>

#if wxUSE_GUI

class wxExEx;
class wxExSTCEntryDialog;

/// Offers variable support to be used in macros.
/// Variables are assigned from an xml node, and
/// you can expand them on an ex component.
class WXDLLIMPEXP_BASE wxExVariable
{
public:
  /// Default constructor.
  wxExVariable();
  
  /// Destructor.
 ~wxExVariable();
  
  /// Constructor using xml node, setting name, value,
  /// prefix using node attributes.
  wxExVariable(const wxXmlNode* node);
  
  /// Expands variable to ex component.
  /// This might update the value, and set the modified flag.
  /// Returns true if variable could be expanded.
  bool Expand(bool playback, wxExEx* ex);
  
  /// Expands variable to value text.
  /// This might update the value, and set the modified flag.
  /// Returns true if variable could be expanded.
  bool Expand(bool playback, wxExEx* ex, wxString& value);
  
  /// Returns variable name.
  const wxString& GetName() const {return m_Name;};
  
  /// Returns true if expanding has modified the value.
  bool IsModified() const {return m_IsModified;};
  
  /// Save in xml node.
  void Save(wxXmlNode* node) const;
private:  
  bool ExpandBuiltIn(wxExEx* ex, wxString& expanded) const;
  bool ExpandInput(bool playback, wxString& expanded);
  bool ExpandTemplate(wxExEx* ex, wxString& expanded);

  bool m_IsModified;
    
  int m_Type;
  
  wxString m_Name;
  wxString m_Prefix;
  
  /// We keep values of input variables,
  /// so, while playing back, you have to enter them only once.
  /// The values are cleared each time you start playback.
  wxString m_Value;
  
  // The dialog used when a prefix is available.
  wxExSTCEntryDialog* m_Dialog;
};

#endif // wxUSE_GUI
#endif
