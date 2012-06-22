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
class WXDLLIMPEXP_BASE wxExVariable
{
public:
  /// Default constructor.
  wxExVariable();
  
  /// Constructor using xml node.
  wxExVariable(const wxXmlNode* node);
  
  /// Clears value, depending on type.
  void Clear();
  
  /// Expands variable to ex component.
  /// Returns true if variable could be expanded.
  bool Expand(bool playback, wxExEx* ex);
  
  /// Returns variable name.
  const wxString& GetName() const {return m_Name;};
  
  /// Returns true if expanding has modified value.
  bool IsModified() const {return m_IsModified;};
  
  /// Save in xml node.
  void Save(wxXmlNode* node) const;
private:  
  bool ExpandBuiltIn(wxExEx* ex, wxString& expanded) const;
  bool ExpandInput(bool playback, wxString& expanded);

  bool m_IsModified;
    
  int m_Type;
  wxString m_Name;
  wxString m_Prefix;
  
  /// We keep values of input variables,
  /// so, while playing back, you have to enter them only once.
  /// The values are cleared each time you start playback.
  wxString m_Value;
  
  wxExSTCEntryDialog* m_Dialog;
};

#endif // wxUSE_GUI
#endif
