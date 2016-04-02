////////////////////////////////////////////////////////////////////////////////
// Name:      vifsm.h
// Purpose:   Declaration of class wxExViFSMEntry and wxExViFSM
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <wx/dlimpexp.h>

/// Contains an entry of FSM table.
class WXDLLIMPEXP_BASE wxExViFSMEntry
{
public:
  /// Constructor, specify current state, key action,
  /// next state because of key, and function to
  /// call just before transition to the next state.
  wxExViFSMEntry(int state, int action, int next, 
    std::function<void(const std::string&)>);
  
  /// Returns key action.
  int Action() const {return m_Action;};
  
  /// Invokes the process and returns next state.
  int Next(const std::string& command);
  
  /// Returns current state.
  int State() const {return m_State;};
private:  
  const int m_State;
  const int m_Action;
  const int m_NextState;
  std::function<void(const std::string&)> m_Process;
};

class wxExVi;

/// Offers vi FSM table for handling vi mode.
class WXDLLIMPEXP_BASE wxExViFSM
{
public:  
  /// Constructor, specify vi component, method
  /// to be called when going into insert mode and back to normal mode.
  wxExViFSM(wxExVi* vi, 
    std::function<void(const std::string&)> insert,
    std::function<void(const std::string&)> normal);
  
  /// Returns insert commands.
  const auto & GetInsertCommands() const {return m_InsertCommands;};

  /// Returns current state.  
  int State() const {return m_State;};
  
  /// Returns state as a string.
  const std::string StateString() const;
  
  /// Transitions to next state depending on command.
  /// Returns true if command represents a state change, otherwise false.
  /// That does not mean that state was changed, in case of readonly doc.
  bool Transition(const std::string& command);
private:  
  wxExVi* m_vi;
  
  int m_State;
  
  std::vector<wxExViFSMEntry> m_FSM;
  std::vector<std::pair<int, std::function<void()>>> m_InsertCommands;
};
