////////////////////////////////////////////////////////////////////////////////
// Name:      property.h
// Purpose:   Declaration of wxExProperty class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXPROPERTY_H
#define _EXPROPERTY_H

#include <wx/xml/xml.h>

class wxStyledTextCtrl;

class WXDLLIMPEXP_BASE wxExProperty
{
public:
  /// Default constructor.
  wxExProperty(const wxXmlNode* node = NULL);

  /// Applies this property to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Resets this property
  void ApplyReset(wxStyledTextCtrl* stc) const;

  /// Returns true if property is valid.
  bool IsOk() const;
private:
  void Set(const wxXmlNode* node);
  
  wxString m_Name;
  wxString m_Value;
};
#endif
