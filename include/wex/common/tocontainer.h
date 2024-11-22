////////////////////////////////////////////////////////////////////////////////
// Name:      to_container.h
// Purpose:   Declaration of wex::to_container class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/tokenizer.hpp>
#include <wx/arrstr.h>
#include <wx/combobox.h>
#include <wx/filedlg.h>
#include <wx/generic/dirctrlg.h>

#define WEX_CONVERT(IN)                                                        \
  if (IN.GetParent() == nullptr)                                               \
    return;                                                                    \
  wxArrayString paths;                                                         \
  in.GetPaths(paths);                                                          \
  from_array_string(paths);

namespace wex
{
/// Offers a class to keep several objects into a templatized container.
template <class T> class to_container
{
public:
  /// Constructor, using std::vector of std::string as input.
  explicit to_container(const std::vector<std::string>& in);

  /// Constructor, using array string as input.
  explicit to_container(const wxArrayString& in) { from_array_string(in); }

  /// Constructor, using file dialog as input.
  /// Fills the container with the full paths of the files chosen.
  explicit to_container(const wxFileDialog& in) { WEX_CONVERT(in); }

  /// Constructor, using generic dirctrl as input.
  /// Fills the container with the currently-selected directory or filename.
  explicit to_container(const wxGenericDirCtrl& in) { WEX_CONVERT(in); }

  /// Constructor, using string as input, each word results in
  /// a container element.
  explicit to_container(
    /// string containing elements
    const std::string& in,
    /// delimiter for elements
    const std::string& delims = " \t\r\n");

  /// Constructor, using a combobox as input.
  /// The current value in the comboxbox is used as first item
  /// in the container, the other items are appended.
  explicit to_container(const wxComboBox* cb, size_t max_items = UINT_MAX);

  /// Returns the container.
  const auto& get() const { return m_container; }

private:
  void from_array_string(const wxArrayString& in);
  void from_combobox(const wxComboBox* in, int start, size_t max_items);

  T m_container;
};

// implementation

template <class T>
to_container<T>::to_container(const std::vector<std::string>& in)
{
  std::copy(in.begin(), in.end(), back_inserter(m_container));
};

template <class T>
to_container<T>::to_container(const std::string& in, const std::string& delims)
{
  boost::tokenizer<boost::char_separator<char>> tok(
    in,
    boost::char_separator<char>(delims.c_str()));

  for (auto it = tok.begin(); it != tok.end(); ++it)
  {
    std::string token(*it);

    // if escape space, add next token
    while (token.back() == '\\')
    {
      if (++it != tok.end())
      {
        token = token.substr(0, token.size() - 1) + " " + *it;
      }
    }

    m_container.emplace_back(token);
  }
}

template <class T>
to_container<T>::to_container(const wxComboBox* cb, size_t max_items)
{
  if (max_items == 0)
  {
    return;
  }

  m_container.emplace_back(cb->GetValue());

  switch (cb->FindString(cb->GetValue(), true)) // case-sensitive
  {
    case 0:
      // The string is already present as the first one, add
      // all other items.
      from_combobox(cb, 1, max_items);
      break;

    case wxNOT_FOUND:
      // Add the string, as it is not in the combobox, to the text,
      // simply by appending all combobox items.
      from_combobox(cb, 0, max_items);
      break;

    default:
      // Reorder. The new first element already present, just add all
      // others.
      for (size_t i = 0; i < cb->GetCount() && i < max_items; i++)
      {
        if (const std::string cb_element(cb->GetString(i));
            cb_element != cb->GetValue())
        {
          m_container.emplace_back(cb_element);
        }
      }
  }
};

template <class T>
void to_container<T>::from_array_string(const wxArrayString& in)
{
  for (const auto& it : in)
  {
    if (!it.empty())
    {
      m_container.emplace_back(it);
    }
  }
}
template <class T>
void to_container<T>::from_combobox(
  const wxComboBox* in,
  int               start,
  size_t            max_items)
{
  for (size_t i = start; i < in->GetCount() && i < max_items; i++)
  {
    m_container.emplace_back(in->GetString(i));
  }
}
}; // namespace wex
