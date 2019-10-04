////////////////////////////////////////////////////////////////////////////////
// Name:      autocomplete.h
// Purpose:   Declaration of class wex::autocomplete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
    bool activate(const std::string& text);

    /// Builds and shows autocomplete lists on the 
    /// stc component. This can be a list
    /// according to CTags, previously inserted text,
    /// or keywords for current lexer.
    bool apply(char c);

    /// Clears filter.
    void reset();

    /// Returns true if active.
    bool use() const;
    
    /// Sets autocomplete on or off.
    void use(bool on) {m_use = on;};
  private:
    void clear();
    bool show_ctags(bool show) const;
    bool show_inserts(bool show) const;
    bool show_keywords(bool show) const;

    const size_t m_min_size;

    bool m_use {true};

    std::string m_text;
    std::set< std:: string > m_inserts;

    ctags_entry m_filter;
    stc* m_stc;
  };
};
