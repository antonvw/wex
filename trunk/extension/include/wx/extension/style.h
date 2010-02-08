////////////////////////////////////////////////////////////////////////////////
// Name:      style.h
// Purpose:   Declaration of wxExStyle class
// Author:    Anton van Wezenbeek
// Created:   2010-02-08
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXSTYLE_H
#define _EXSTYLE_H

#include <wx/xml/xml.h>

class wxStyledTextCtrl;

class wxExStyle
{
public:
  /// Default constructor.
  wxExStyle(const wxXmlNode* node = NULL);

  /// Constructor using no and value.
  wxExStyle(const wxString& no, const wxString& value);

  /// Applies this style to stc component.
  void Apply(wxStyledTextCtrl* stc) const;

  /// Gets the no.
  const wxString& GetNo() const {return m_No;};
private:
  void Set(const wxXmlNode* node);

  wxString m_No;
  wxString m_Value;
};
#endif
