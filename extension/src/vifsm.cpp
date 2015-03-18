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

#define EMPTY [](const std::string& command){;}

#define MAKE_ENTRY( STATE, ACTION, NEXT, PROCESS )                 \
  m_FSM.push_back(                                                 \
    wxExViFSMEntry(wxExVi::STATE, ACTION, wxExVi::NEXT, PROCESS)); \

wxExViFSM::wxExViFSM(wxExVi* vi, 
  std::function<void(const std::string&)> insert,
  std::function<void(const std::string&)> normal)
  : m_vi(vi)
  , m_State(wxExVi::MODE_NORMAL)
{
   MAKE_ENTRY ( MODE_NORMAL,      KEY_INSERT,      MODE_INSERT,      insert)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL,      MODE_VISUAL,      EMPTY)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL_LINE, MODE_VISUAL_LINE, EMPTY)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL_RECT, MODE_VISUAL_RECT, EMPTY)
   MAKE_ENTRY ( MODE_INSERT,      KEY_ESCAPE,      MODE_NORMAL,      normal)
   MAKE_ENTRY ( MODE_INSERT_RECT, KEY_ESCAPE,      MODE_VISUAL_RECT, EMPTY)
   MAKE_ENTRY ( MODE_VISUAL,      KEY_ESCAPE,      MODE_NORMAL,      normal)
   MAKE_ENTRY ( MODE_VISUAL_LINE, KEY_ESCAPE,      MODE_NORMAL,      normal)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_ESCAPE,      MODE_NORMAL,      normal)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_INSERT,      MODE_INSERT_RECT, EMPTY)
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
      fsm.Process(command);

      return true;
    }
  }
  
  return false;
}

wxExViFSMEntry::wxExViFSMEntry(int state, int action, int next, 
  std::function<void(const std::string&)> process)
  : m_State(state)
  , m_Action(action)
  , m_NextState(next)
  , m_Process(process)
{
}

std::function<void(const std::string&)> & wxExViFSMEntry::Process(const std::string& command)
{
  m_Process(command);
}
