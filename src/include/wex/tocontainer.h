////////////////////////////////////////////////////////////////////////////////
// Name:      to_container.h
// Purpose:   Declaration of wex::to_container class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/arrstr.h>
#include <wx/combobox.h>
#include <wx/filedlg.h>
#include <wx/generic/dirctrlg.h>
#include <wex/tokenizer.h>

#define WEX_CONVERT( IN )                 \
  if (IN.GetParent() == nullptr) return;  \
  wxArrayString paths;                    \
  in.GetPaths(paths);                     \
  FromArrayString(paths);

namespace wex
{
  /// Offers a class to keep several objects into a templatized container.
  template <class T> 
  class to_container
  {
  public:  
    /// Constructor, using array string.
    to_container(const wxArrayString& in) {
      FromArrayString(in);};
    
    /// Constructor, using file dialog.
    /// Fills the container with the full paths of the files chosen.
    to_container(const wxFileDialog& in) {
      WEX_CONVERT(in);}

    /// Constructor, using generic dirctrl.
    /// Fills the container with the currently-selected directory or filename. 
    to_container(const wxGenericDirCtrl& in) {
      WEX_CONVERT(in);}
    
    /// Constructor, using string, each word results in a container element.
    to_container(
      /// string containing elements
      const std::string& in,
      /// delimiter for elements
      const std::string& delims = " \t\r\n") {
      tokenizer tkz(in, delims);
      while (tkz.has_more_tokens())
      {
        std::string token(tkz.get_next_token());
        // if escape space, add next token
        if (token.back() == '\\')
        {
          token = token.substr(0, token.size() - 1) + " " + tkz.get_next_token();
        }
        m_Container.emplace_back(token);
      }}

    /// Constructor, using a combobox.
    to_container(const wxComboBox* cb, size_t max_items = UINT_MAX) {
      if (max_items == 0) return;
      // wxArrayString has no emplace_back.
      if (!cb->GetValue().empty())
      {
        m_Container.push_back(cb->GetValue().ToStdString());
        switch (cb->FindString(cb->GetValue(), true)) // case sensitive
        {
          case 0: 
            // The string is already present as the first one, add
            // all other items.
            for (size_t i = 1; i < cb->GetCount() && i < max_items; i++)
              m_Container.push_back(cb->GetString(i).ToStdString());
            break;
          case wxNOT_FOUND:
            // Add the string, as it is not in the combobox, to the text,
            // simply by appending all combobox items.
            for (size_t i = 0; i < cb->GetCount() && i < max_items; i++)
              m_Container.push_back(cb->GetString(i).ToStdString());
          break;
          default:
            // Reorder. The new first element already present, just add all others.
            for (size_t i = 0; i < cb->GetCount() && i < max_items; i++)
            {
              const std::string cb_element(cb->GetString(i));
              if (cb_element != cb->GetValue())
                m_Container.push_back(cb_element);
            }
        }
      }
      else
      {
        for (size_t i = 0; i < cb->GetCount() && i < max_items; i++)
          m_Container.push_back(cb->GetString(i).ToStdString());
      }};
    
    /// Returns the container.
    const auto & get() const {return m_Container;};
  private:
    void FromArrayString(const wxArrayString& in) {
      for (const auto& it : in)
      {
        m_Container.emplace_back(it);
      }}
    
    T m_Container;
  };
};
