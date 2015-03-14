////////////////////////////////////////////////////////////////////////////////
// Name:      vifsm.cpp
// Purpose:   Implementation of class wxExViFSM
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/vifsm.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>

enum
{
  KEY_INSERT,
  KEY_VISUAL,
  KEY_VISUAL_LINE,
  KEY_VISUAL_RECT,
  KEY_ESCAPE
};

#define MAKE_ENTRY( STATE, ACTION, NEXT )                               \
  m_FSM.push_back(wxExViFSMEntry(wxExVi::STATE, ACTION, wxExVi::NEXT)); \

wxExViFSM::wxExViFSM(wxExVi* vi, 
  int state, std::function<void(const std::string&)> process)
  : m_vi(vi)
  , m_State(state)
  , m_Process(process)
{
   MAKE_ENTRY ( MODE_NORMAL,      KEY_INSERT,      MODE_INSERT)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL,      MODE_VISUAL)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL_LINE, MODE_VISUAL_LINE)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL_RECT, MODE_VISUAL_RECT)
   MAKE_ENTRY ( MODE_INSERT,      KEY_ESCAPE,      MODE_NORMAL)
   MAKE_ENTRY ( MODE_INSERT_RECT, KEY_ESCAPE,      MODE_VISUAL_RECT)
   MAKE_ENTRY ( MODE_VISUAL,      KEY_ESCAPE,      MODE_NORMAL)
   MAKE_ENTRY ( MODE_VISUAL_LINE, KEY_ESCAPE,      MODE_NORMAL)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_ESCAPE,      MODE_NORMAL)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_INSERT,      MODE_INSERT_RECT)
}

bool wxExViFSM::Transition(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }
    
  int key = 0;
  
  if (command.size() == 1)
  {
    switch (command[0])
    {
      case 'F': key = KEY_VISUAL_RECT; break;
      case 'v': key = KEY_VISUAL; break;
      case 'V': key = KEY_VISUAL_LINE; break;
      case 27:  key = KEY_ESCAPE; break;
      case 'a': 
      case 'i': 
      case 'o': 
      case 'A': 
      case 'C': 
      case 'I': 
      case 'O': 
      case 'R': key = KEY_INSERT; break;
      default: return false;
    }
  }
  else
  {
    if (command == "cc" || command == "cw")
    {
      key = KEY_INSERT;
    }
    else
    {
      return false;
    }
  }
  
  for (auto fsm : m_FSM)
  {
    if (fsm.State() == m_State && fsm.Action() == key)
    {
      if (key == KEY_INSERT &&
        (m_vi->GetSTC()->GetReadOnly() || m_vi->GetSTC()->HexMode()))
      {
        return true;
      }
  
      m_State = fsm.Next();
      
      if (m_State == wxExVi::MODE_INSERT)
      {
        m_Process(command);
      }
      
      return true;
    }
  }
  
  return false;
}

wxExViFSMEntry::wxExViFSMEntry(int state, int action, int next)
  : m_State(state)
  , m_Action(action)
  , m_NextState(next)
{
}
