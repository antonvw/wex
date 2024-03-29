////////////////////////////////////////////////////////////////////////////////
// Name:      auto_complete.h
// Purpose:   Declaration of class wex::auto_complete
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <string>

namespace wex
{
class scope;
class stc;

/// Offers a class for auto completion on wex::stc.
class auto_complete
{
public:
  /// Constructor, provide stc to trigger completion.
  explicit auto_complete(stc* stc);

  /// Destructor.
  ~auto_complete();

  /// Clears data.
  void clear();

  /// Completes the auto completed text.
  /// This might setup a filter for next
  /// auto complete list.
  bool complete(const std::string& text);

  /// Returns current insert.
  /// (e.g. a variable or a class name)
  const auto& insert() const { return m_insert; }

  /// Returns all inserts (independent of scope)
  const auto& inserts() const { return m_inserts; }

  /// Builds and shows auto complete lists on the
  /// stc component. This can be a list
  /// according to CTags, previously inserted text,
  /// or keywords for current lexer.
  /// Returns true if show was invoked.
  bool on_char(char c);

  /// Takes care that auto complete scope is updated
  /// with level information on current stc position.
  /// Returns true if update was done.
  bool sync() const;

  /// Returns true if active.
  bool use() const;

  /// Sets auto completion on or off.
  void use(bool on) { m_use = on; }

  /// Returns the class name of variable for current level (scope).
  const std::string variable(const std::string& name) const;

private:
  class actions;

  bool action_back();
  bool action_default(char c);
  bool action_request(char c, actions& a);

  bool determine_actions(char c, actions& a);
  bool show_ctags();
  bool show_inserts(bool show) const;
  bool show_keywords(bool show) const;
  void store_variable();
  void update_inserts();

  bool m_use{true};

  std::string m_active, m_insert, m_request_members;

  std::set<std::string> m_inserts;

  scope* m_scope;
  stc*   m_stc;
};
}; // namespace wex
