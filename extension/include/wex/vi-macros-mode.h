////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-mode.h
// Purpose:   Implementation of class wex::vi_macros_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

namespace wex
{
  class ex;
  class variable;
  class vi_macros_fsm;

  /// Offers the vi macros mode, like playing back or recording.
  class vi_macros_mode
  {
  public:
    /// Default constructor.
    vi_macros_mode();

    /// Destructor.
   ~vi_macros_mode();

    /// Expands template variable.
    /// Returns true if the template file name exists,
    /// and all variables in it could be expanded.
    bool expand(
      /// ex component to use
      ex* ex, 
      /// variable (containing template file name)
      const variable& variable, 
      /// value to receive contents
      std::string& expanded);

    /// Are we playing back?
    bool is_playback() const;

    /// Are we recording?
    bool is_recording() const;

    /// Returns the mode as a string.
    const std::string string() const;

    /// transitions between modes.
    /// If command starts with:
    /// q: Starts recording a macro (appends to 
    ///    existing macro if macro is single upper case character).
    /// @: Playsback the macro.
    /// Returns number of characters processed from command.
    int transition(
      /// macro name after first character
      const std::string& command, 
      /// ex component to use, required in case of playback
      ex* ex = nullptr, 
      /// is the command complete
      bool complete = false,
      /// number of times this maco should be executed, in case of playback
      int repeat = 1);
  private:
    vi_macros_fsm* m_FSM;
  };
};
