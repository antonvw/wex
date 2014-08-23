////////////////////////////////////////////////////////////////////////////////
// Name:      variable.h
// Purpose:   Declaration of class wxExVariable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVARIABLE_H
#define _EXVARIABLE_H

#if wxUSE_GUI

class wxExEx;
class wxExSTCEntryDialog;
class wxXmlNode;

/// Offers variable support to be used in macros.
/// Variables are assigned from an xml node, and
/// you can expand them on an ex component.
class WXDLLIMPEXP_BASE wxExVariable
{
public:
  /// Default constructor.
  wxExVariable();
  
  /// Constructor using name.
  wxExVariable(const wxString& name);
  
  /// Constructor using xml node, setting name, type, value,
  /// prefix using node attributes.
  wxExVariable(const wxXmlNode* node);
  
  /// Sets the ask for input member, if appropriate for type.
  void AskForInput();
  
  /// Expands variable to ex component.
  /// This might update the internal value, and set the modified flag.
  /// Returns true if variable could be expanded.
  bool Expand(wxExEx* ex);
  
  /// Expands variable to value text.
  /// This might update the internal value, and set the modified flag.
  /// Returns true if variable could be expanded.
  bool Expand(wxExEx* ex, wxString& value);
  
  /// Returns variable name.
  const wxString& GetName() const {return m_Name;};
  
  /// Returns variable value.
  const wxString& GetValue() const {return m_Value;};
  
  /// Returns true if this variable is of type input.
  bool IsInput() const;
  
  /// Returns true if expanding has modified the value.
  bool IsModified() const {return m_IsModified;};
  
  /// Save in xml node.
  void Save(wxXmlNode* node) const;
  
  /// Resets the ask for input member, if appropriate for type.
  void SkipInput();
private:  
  bool ExpandBuiltIn(wxExEx* ex, wxString& expanded) const;
  bool ExpandInput(wxString& expanded);

  bool m_AskForInput;
  bool m_IsModified;
    
  int m_Type;
  
  wxString m_Name;
  wxString m_Prefix;
  
  /// We keep values of input variables,
  /// so, while playing back, you have to enter them only once.
  /// The m_AskForInput member is set each time you start playback.
  wxString m_Value;
  
  // The dialog used.
  static wxExSTCEntryDialog* m_Dialog;
};

#endif // wxUSE_GUI
#endif
