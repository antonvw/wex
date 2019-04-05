////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.h
// Purpose:   Declaration of wex::textctrl_input class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <wx/textctrl.h>
#include <wex/ex-command.h>

namespace wex
{
  /// Offers a class to relate text control to values with iterators.
  class textctrl_input
  {
  public:
    /// Constructor, fills values from config.
    textctrl_input(ex_command::type_t type);
    
    /// Destructor, writes values to config.
   ~textctrl_input();

    /// Returns value on the list pointed to by iterator, 
    /// or empty string, if iterator is at end.
    const std::string get() const;
    
    /// Sets first value on the list.
    /// Sets iterator to begin of list.
    /// Returns false if value is empty.
    bool set(const std::string& value);
    
    /// Sets first value on the list from specified text control.
    bool set(const wxTextCtrl* tc) {return set(tc->GetValue().ToStdString());};

    /// Sets iterator according to specified key, and then
    /// sets value of text control (if not nullptr) to the list value 
    /// related to iterator.
    /// Returns false if current list is empty, or key not ok.
    bool set(
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
    void set(const std::list < std::string > & values);

    /// Returns type.
    auto type() const {return m_Type;};

    /// Gets all values.
    const auto& values() const {return m_Values;};
  private:
    const ex_command::type_t m_Type;
    const std::string m_Name;
    std::list < std::string > m_Values;
    std::list < std::string >::const_iterator m_Iterator;
  };
};
