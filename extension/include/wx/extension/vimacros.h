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
  
  /// Is macro available.
  bool Contains(const wxString& macro) const;
  
  /// Returns macro contents.
  const wxString& Get(const wxString& macro) const;
  
  /// Returns all macros as an array of strings.
  const wxArrayString Get() const;
  
  /// Returns separator.
  const char GetSeparator() const {return m_Separator;};
  
  /// Loads all macros from xml document.
  static void LoadDocument();
  
  /// Saves all macros to xml document.
  static void SaveDocument();
private:  
  const wxFileName GetFileName() const;
  bool Load(wxXmlDocument& doc);
    
  std::map <wxString, wxString> m_Macros;
  const char m_Separator;
};
#endif // wxUSE_GUI
#endif
