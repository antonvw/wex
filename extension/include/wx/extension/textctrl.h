////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.h
// Purpose:   Declaration of wxExTextCtrlInput class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <wx/textctrl.h>

/// Offers a class to relate text control to values with iterators.
class WXDLLIMPEXP_BASE wxExTextCtrlInput
{
public:
  /// Constructor.
  wxExTextCtrlInput(const wxString& name);
  
  /// Destructor.
 ~wxExTextCtrlInput();

  /// Returns value on the list pointed to by iterator, 
  /// or empty string, if iterator is at end.
  const wxString Get() const;
  
  /// Gets all values.
  const auto& GetValues() const {return m_Values;};

  /// Sets first value on the list.
  /// Sets iterator to begin of list.
  /// Returns false if value is empty.
  bool Set(const wxString& value);
  
  /// Sets first value on the list from specified text control.
  bool Set(const wxTextCtrl* tc) {return Set(tc->GetValue());};

  /// Sets iterator according to specified key, and then
  /// sets value of text control (if not nullptr) to the list value 
  /// related to iterator.
  /// Returns false if current list is empty, or key not ok.
  bool Set(
    /// the key:
    /// - WXK_UP
    /// - WKK_DOWN
    /// - WXK_HOME
    /// - WXK_END
    /// - WXK_PAGEUP
    /// - WXK_PAGEDOWN
    int key,
    /// the text control
    wxTextCtrl* tc = nullptr); 
  
  /// Sets all values (values might be empty).
  /// Sets iterator to begin of list.
  void Set(const std::list < wxString > & values);
private:
  std::list < wxString > m_Values;
  std::list < wxString >::const_iterator m_Iterator;
  const wxString m_Name;
};
