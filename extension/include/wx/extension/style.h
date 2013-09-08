////////////////////////////////////////////////////////////////////////////////
// Name:      style.h
// Purpose:   Declaration of wxExStyle class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTYLE_H
#define _EXSTYLE_H

#include <set>

class wxXmlNode;
class wxStyledTextCtrl;

/// This class defines our scintilla styles. The no as in xml or in the string
/// can be a single style, or several styles separated by a comma.
/// E.g.
/// 1,2,3=fore:light steel blue,italic,size:8
/// 1,2,3 are the scintilla styles numbers, and the rest is spec
class WXDLLIMPEXP_BASE wxExStyle
{
public:
  /// Default constructor.
  wxExStyle() {;};
  
  /// Constructor using xml node (sets no from the no attribute).
  wxExStyle(const wxXmlNode* node, const wxString& macro);

  /// Constructor using no and value.
  wxExStyle(
    const wxString& no, 
    const wxString& value,
    const wxString& macro = "global");

  /// Applies this style to stc component.
  /// If no style is present, STC StyleResetDefault is invoked.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Is the default style part of these styles.
  bool ContainsDefaultStyle() const;

  /// Returns true if this style is valid.
  bool IsOk() const;
private:
  void Set(const wxXmlNode* node, const wxString& macro);
  void SetNo(const wxString& no, const wxString& macro);

  std::set <int> m_No;
  wxString m_Value;
};
#endif
