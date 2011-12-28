////////////////////////////////////////////////////////////////////////////////
// Name:      vimacros.h
// Purpose:   Declaration of class wxExViMacros
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXVIMACROS_H
#define _EXVIMACROS_H

#include <map>
#include <wx/filename.h>
#include <wx/xml/xml.h>

#if wxUSE_GUI

/// Offers the macro collection.
class WXDLLIMPEXP_BASE wxExViMacros
{
public:  
  /// Default constructor.
  wxExViMacros();
  
  /// Adds single char to macro.
  void Add(const wxString& macro, char c, bool separated = false);
  
  /// Adds (separated) text to macro.
  void Add(const wxString& macro, const wxString& text);
  
  /// Adds a separator only.
  void AddSeparator(const wxString& macro);

  /// Clears macro.
  void Clear(const wxString& macro);
  
  /// Returns macro contents.
  const wxString Get(const wxString& macro) const;
  
  /// Returns all macros as an array of strings.
  const wxArrayString Get() const;
  
  /// Returns separator.
  const char GetSeparator() const {return m_Separator;};
  
  /// Is macro available.
  bool IsAvailable(const wxString& macro =wxEmptyString) const;
  
  /// Loads all macros from xml document.
  static void LoadDocument();
  
  /// Saves all macros to xml document.
  static void SaveDocument();
private:  
  static const wxFileName GetFileName();
  static bool Load(wxXmlDocument& doc);
    
  static std::map <wxString, wxString> m_Macros;
  const char m_Separator;
};
#endif // wxUSE_GUI
#endif
