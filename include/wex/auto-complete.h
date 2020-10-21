////////////////////////////////////////////////////////////////////////////////
// Name:      auto_complete.h
// Purpose:   Declaration of class wex::auto_complete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <string>
#include <wex/ctags-entry.h>

namespace wex
{
  class scope;
  class stc;

  /// Offers a class for auto completion on wex::stc.
  class auto_complete
  {
  public:
    /// Constructor.
    auto_complete(stc* stc);

    /// Destructor.
    ~auto_complete();

    /// Activates the auto completed text.
    /// This might setup a filter for next
    /// auto complete list.
    bool activate(const std::string& text);

    /// Clears filter and text.
    void clear();

    /// Builds and shows auto complete lists on the
    /// stc component. This can be a list
    /// according to CTags, previously inserted text,
    /// or keywords for current lexer.
    /// Returns true if show was invoked.
    bool on_char(char c);

    /// Takes care that auto complete scope is updated
    /// with level information on current stc position.
    void sync() const;

    /// Returns true if active.
    bool use() const;

    /// Sets auto completion on or off.
    void use(bool on) { m_use = on; };

  private:
    bool show_ctags(bool show);
    bool show_inserts(bool show) const;
    bool show_keywords(bool show) const;

    const size_t m_min_size;

    bool m_get_members{false}, m_use{true};

    std::string           m_active, m_text;
    std::set<std::string> m_inserts;

    scope* m_scope;
    stc*   m_stc;
  };
}; // namespace wex
