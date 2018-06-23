////////////////////////////////////////////////////////////////////////////////
// Name:      autocomplete.h
// Purpose:   Declaration of class wxExAutoComplete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <string>
#include <wx/dlimpexp.h>
#include <wx/extension/ctags-entry.h>

class wxExSTC;

/// Offers a class for autocompletion on wxExSTC.
class WXDLLIMPEXP_BASE wxExAutoComplete
{
public:
  /// Constructor.
  wxExAutoComplete(wxExSTC* stc);

  /// Activates the autocompleted text.
  /// This might setup a filter for next
  /// autocomplete list.
  bool Activate(const std::string& text);

  /// Builds and shows autocomplete lists on the 
  /// STC component. This can be a list
  /// according to CTags, previously inserted text,
  /// or keywords for current lexer.
  bool Apply(char c);

  /// Clears filter.
  void Reset();

  /// Sets autocomplete on or off.
  /// Default on.
  void Use(bool use) {m_Use = use;};
private:
  void Clear();
  bool ShowCTags(bool show) const;
  bool ShowInserts(bool show) const;
  bool ShowKeywords(bool show) const;
  bool Use() const;

  const size_t m_MinSize;

  bool m_Use {true};

  std::string m_Text;
  std::set< std:: string > m_Inserts;

  wxExCTagsEntry m_Filter;
  wxExSTC* m_STC;
};
