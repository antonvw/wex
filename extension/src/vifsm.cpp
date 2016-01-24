////////////////////////////////////////////////////////////////////////////////
// Name:      vifsm.cpp
// Purpose:   Implementation of class wxExViFSM
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
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

#define MAKE_ENTRY( STATE, ACTION, NEXT, PROCESS )                 \
  m_FSM.push_back(                                                 \
    wxExViFSMEntry(wxExVi::STATE, ACTION, wxExVi::NEXT, PROCESS)); \

#define NAVIGATE(SCOPE, DIRECTION)                                 \
  m_vi->GetSTC()->SCOPE##DIRECTION();                              \
  
wxExViFSM::wxExViFSM(wxExVi* vi, 
  std::function<void(const std::string&)> insert,
  std::function<void(const std::string&)> normal)
  : m_vi(vi)
  , m_State(wxExVi::MODE_NORMAL)
{
   MAKE_ENTRY ( MODE_NORMAL,      KEY_INSERT,      MODE_INSERT,      insert)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL,      MODE_VISUAL,      nullptr)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL_LINE, MODE_VISUAL_LINE, nullptr)
   MAKE_ENTRY ( MODE_NORMAL,      KEY_VISUAL_RECT, MODE_VISUAL_RECT, nullptr)
   MAKE_ENTRY ( MODE_INSERT,      KEY_ESCAPE,      MODE_NORMAL,      normal)
   MAKE_ENTRY ( MODE_INSERT_RECT, KEY_ESCAPE,      MODE_VISUAL_RECT, nullptr)
   MAKE_ENTRY ( MODE_VISUAL,      KEY_INSERT,      MODE_INSERT,      insert)
   MAKE_ENTRY ( MODE_VISUAL,      KEY_ESCAPE,      MODE_NORMAL,      nullptr)
   MAKE_ENTRY ( MODE_VISUAL,      KEY_VISUAL_LINE, MODE_VISUAL_LINE, nullptr)
   MAKE_ENTRY ( MODE_VISUAL,      KEY_VISUAL_RECT, MODE_VISUAL_RECT, nullptr)
   MAKE_ENTRY ( MODE_VISUAL_LINE, KEY_ESCAPE,      MODE_NORMAL,      nullptr)
   MAKE_ENTRY ( MODE_VISUAL_LINE, KEY_VISUAL,      MODE_VISUAL,      nullptr)
   MAKE_ENTRY ( MODE_VISUAL_LINE, KEY_VISUAL_RECT, MODE_VISUAL_RECT, nullptr)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_INSERT,      MODE_INSERT_RECT, insert)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_ESCAPE,      MODE_NORMAL,      nullptr)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_VISUAL,      MODE_VISUAL,      nullptr)
   MAKE_ENTRY ( MODE_VISUAL_RECT, KEY_VISUAL_LINE, MODE_VISUAL_LINE, nullptr)
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
      case 'K': key = KEY_VISUAL_RECT; break;
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
    if (command[0] == 'c')
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
  
      m_State = fsm.Next(command);
      
      switch (m_State)
      {
        case wxExVi::MODE_INSERT:
          switch ((int)command[0])
          {
            case 'a': NAVIGATE(Char, Right); break;
            case 'A': NAVIGATE(Line, End); break;
            case 'R': m_vi->GetSTC()->SetOvertype(true); break;
            case 'I': NAVIGATE(Line, Home); break;
            case 'o': 
              NAVIGATE(Line, End);
              m_vi->GetSTC()->NewLine(); 
              break;
            case 'C': 
              m_vi->GetSTC()->LineEndExtend();
              m_vi->Cut();
              break;
            case 'O': 
              NAVIGATE(Line, Home); 
              m_vi->GetSTC()->NewLine(); 
              NAVIGATE(Line, Up); 
              break;
          }
          break;
          
        case wxExVi::MODE_VISUAL_LINE:
          if (m_vi->GetSTC()->SelectionIsRectangle())
          {
            m_vi->GetSTC()->Home();
            m_vi->GetSTC()->LineDownExtend();
          }
          else if (m_vi->GetSTC()->GetSelectedText().empty())
          {
            m_vi->GetSTC()->Home();
            m_vi->GetSTC()->LineDownExtend();
          }
          else
          {
            const int selstart = m_vi->GetSTC()->PositionFromLine(m_vi->GetSTC()->LineFromPosition(m_vi->GetSTC()->GetSelectionStart()));
            const int selend = m_vi->GetSTC()->PositionFromLine(m_vi->GetSTC()->LineFromPosition(m_vi->GetSTC()->GetSelectionEnd()) + 1);
            m_vi->GetSTC()->SetSelection(selstart, selend);
            m_vi->GetSTC()->HomeExtend();
          }
          break;
      }
      
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

int wxExViFSMEntry::Next(const std::string& command)
{
  if (m_Process != nullptr)
  {
    m_Process(command);
  }

  return m_NextState;
}
