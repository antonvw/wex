////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.h
// Purpose:   Declaration of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

class wxTextEntry;

namespace wex
{
namespace factory
{
class stc;
};

/// Offers a command to be used by ex command.
class ex_command
{
public:
  /// The type of ex command.
  enum class type_t
  {
    // these types are indexed in textctrl_imp
    COMMAND,       ///< a normal (ex) command (:)
    COMMAND_RANGE, ///< a normal (ex) command on a range (:'<,'>)
    CALC,          ///< a calculation command (control-r =)
    COMMAND_EX,    ///< a normal (ex) command, entered in explicit ex mode (:)
    ESCAPE,        ///< an escape command (!)
    ESCAPE_RANGE,  ///< an escape command on a range (:'<,'>!)
    FIND_MARGIN,   ///< a find command (/ or ?) entered on text margin

    // the rest
    NONE,    ///< an empty command
    FIND,    ///< a find command (/ or ?)
    REPLACE, ///< a replace command (no ex command)
    VI,      ///< a vi command (no ex command)
  };

  /// Returns string used to retrieve addressrange for selection.
  static const std::string selection_range();

  /// Default constructor.
  ex_command();

  /// Constructor, sets stc component.
  explicit ex_command(factory::stc* stc);

  /// Constructor, sets command text.
  explicit ex_command(const std::string& text);

  /// Copy constructor.
  ex_command(const ex_command& c);

  /// Assignment operator.
  ex_command& operator=(const ex_command& c);

  /// Appends a char.
  ex_command& append(char c)
  {
    m_text.push_back(c);
    return *this;
  };

  /// Appends a string.
  ex_command& append(const std::string& s)
  {
    m_text += s;
    return *this;
  };

  /// Appends a char and tries to execute.
  bool append_exec(char c);

  /// Returns last char of command.
  auto back() const { return m_text.back(); }

  /// Clears command text.
  void clear() { m_text.clear(); }

  /// Returns the command text.
  const auto& command() const { return m_text; }

  /// Returns true if command text is empty.
  auto empty() const { return m_text.empty(); }

  /// Erases element(s) at position from command text.
  void erase(size_t pos, size_t len = 1);

  /// Executes the command on the stc component if available.
  bool exec() const;

  /// Returns front of command text.
  auto front() const { return m_text.front(); }

  /// Returns stc component.
  auto* get_stc() const { return m_stc; }

  /// Handles keycode from textentry component.
  void handle(const wxTextEntry* te, int keycode);

  /// Inserts char at pos.
  void insert(size_t pos, char c);

  /// Inserts string at pos.
  void insert(size_t pos, const std::string& s);

  /// Sets to no type.
  void no_type();

  /// Removes last char of command text.
  void pop_back() { m_text.pop_back(); }

  /// Resets command to text (but keeps type).
  ex_command& reset(
    // new text
    const std::string& text = std::string(),
    // a full resets also reset stc components
    bool full = false);

  /// Restores values, if possible from original stc.
  void restore(const ex_command& c);

  /// Sets command text.
  /// The text should start with a command prefix,
  /// like ':' to return the command type.
  ex_command& set(const std::string& text);

  /// Sets new command, except original stc.
  /// This can be used in combination with restore.
  void set(const ex_command& c);

  /// Returns size of command.
  auto size() const { return m_text.size(); }

  /// Returns string type of command.
  std::string str() const;

  /// Returns type of command.
  type_t type() const;

private:
  std::string m_text;

  bool m_has_type{true};

  wex::factory::stc *m_stc{nullptr}, *m_stc_original{nullptr};
};
} // namespace wex
