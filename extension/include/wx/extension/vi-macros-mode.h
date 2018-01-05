////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-mode.h
// Purpose:   Implementation of class wxExViMacrosMode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

class wxExEx;
class wxExVariable;
class wxExViMacrosFSM;

/// Offers the vi macros mode, like playing back or recording.
class wxExViMacrosMode
{
public:
  /// Default constructor.
  wxExViMacrosMode();

  /// Destructor.
 ~wxExViMacrosMode();

  /// Expands template variable.
  /// Returns true if the template file name exists,
  /// and all variables in it could be expanded.
  bool Expand(
    /// ex component to use
    wxExEx* ex, 
    /// variable (containing template file name)
    const wxExVariable& variable, 
    /// value to receive contents
    std::string& expanded);

  /// Are we playing back?
  bool IsPlayback() const;

  /// Are we recording?
  bool IsRecording() const;

  /// Returns the mode as a string.
  const std::string String() const;

  /// Transitions between modes.
  /// If command starts with:
  /// q: Starts recording a macro (appends to 
  ///    existing macro if macro is single upper case character).
  /// @: Playsback the macro.
  /// Returns number of characters processed from command.
  int Transition(
    /// macro name after first character
    const std::string& command, 
    /// ex component to use, required in case of playback
    wxExEx* ex = nullptr, 
    /// is the command complete
    bool complete = false,
    /// number of times this maco should be executed, in case of playback
    int repeat = 1);
private:
  wxExViMacrosFSM* m_FSM;
};
