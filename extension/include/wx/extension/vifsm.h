////////////////////////////////////////////////////////////////////////////////
// Name:      vifsm.h
// Purpose:   Declaration of class wxExViFSMEntry and wxExViFSM
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <vector>
#include <wx/dlimpexp.h>

/// Contains entry of FSM table.
class WXDLLIMPEXP_BASE wxExViFSMEntry
{
public:
  /// Constructor.
  wxExViFSMEntry(int state, int action, int next);
  
  int Action() const {return m_Action;};
  int Next() const {return m_NextState;};
  int State() const {return m_State;};
private:  
  const int m_State;
  const int m_Action;
  const int m_NextState;
};

class wxExVi;

/// Offers vi FSM table for handling vi mode.
class WXDLLIMPEXP_BASE wxExViFSM
{
public:  
  /// Constructor, specify vi component, initial state, method
  /// to be called when going into insert mode.
  wxExViFSM(wxExVi* vi, int state, std::function<void(const std::string&)> process);
  
  /// Returns current state.  
  int State() const {return m_State;};
  
  /// Transitions to next state depending on command.
  /// Returns true if state changed, otherwise false.
  bool Transition(const std::string& command);
private:  
  wxExVi* m_vi;
  
  int m_State;
  
  std::function<void(const std::string&)> m_Process;
  const std::vector<wxExViFSMEntry> m_FSM;
};
