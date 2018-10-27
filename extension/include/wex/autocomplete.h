////////////////////////////////////////////////////////////////////////////////
// Name:      autocomplete.h
// Purpose:   Declaration of class wex::autocomplete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <string>
#include <wex/ctags-entry.h>

namespace wex
{
  class stc;

  /// Offers a class for autocompletion on wex::stc.
  class autocomplete
  {
  public:
    /// Constructor.
    autocomplete(stc* stc);

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

    ctags_entry m_Filter;
    stc* m_STC;
  };
};
