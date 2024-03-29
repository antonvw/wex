////////////////////////////////////////////////////////////////////////////////
// Name:      macro-mode.h
// Purpose:   Implementation of class wex::macro_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <optional>
#include <string>

namespace wex
{
class ex;
class macro_fsm;
class macros;
class variable;

/// Offers the macro mode, like playing back or recording,
/// and the current macro that is recorded or was played back.
class macro_mode
{
public:
  /// Constructor.
  /// The macros specified is the collection of macros
  /// used for replay or record.
  /// This collection might be changed depending on the mode.
  macro_mode(macros* macros);

  /// Destructor.
  ~macro_mode();

  /// Expands template variable.
  /// Returns true if the template file name exists,
  /// and all variables in it could be expanded.
  bool expand(
    /// ex component to use
    ex* ex,
    /// variable (containing template file name)
    const variable& variable,
    /// value to receive contents
    std::string& expanded) const;

  /// Returns current macro.
  const std::string get_macro() const;

  /// Returns the macros collection.
  auto* get_macros() { return m_macros; }

  /// Are we playing back?
  bool is_playback() const;

  /// Are we recording?
  bool is_recording() const;

  /// Returns the mode as a string.
  const std::string str() const;

  /// transitions between modes.
  /// If command starts with:
  /// q: Starts recording a macro (appends to
  ///    existing macro if macro is single upper case character).
  /// \@: Playsback the macro.
  /// Returns number of characters processed from command.
  size_t transition(
    /// macro name after first character
    const std::string& command,
    /// ex component to use, required in case of playback
    ex* ex = nullptr,
    /// is the command complete
    bool complete = false,
    /// number of times this macro should be executed, in case of playback
    size_t repeat = 1);

private:
  std::optional<size_t>
       transition_at(std::string& macro, ex* ex, bool complete, size_t repeat);
  bool transition_q(std::string& macro, ex* ex, bool complete);

  macro_fsm* m_fsm;
  macros*    m_macros;
};
}; // namespace wex
