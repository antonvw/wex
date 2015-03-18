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

/// Contains an entry of FSM table.
class WXDLLIMPEXP_BASE wxExViFSMEntry
{
public:
  /// Constructor, specify current state, key action,
  /// next state because of key, and function to
  /// call after transition to the next state.
  wxExViFSMEntry(int state, int action, int next, 
    std::function<void(const std::string&)>);
  
  int Action() const {return m_Action;};
  int Next() const {return m_NextState;};
  int State() const {return m_State;};
  std::function<void(const std::string&)> & Process(const std::string& command);
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
  
  /// Returns current state.  
  int State() const {return m_State;};
  
  /// Transitions to next state depending on command.
  /// Returns true if command represents a state change, otherwise false.
  /// That does not mean that state was changed, in case of readonly doc.
  bool Transition(const std::string& command);
private:  
  wxExVi* m_vi;
  
  int m_State;
  
  std::vector<wxExViFSMEntry> m_FSM;
};
