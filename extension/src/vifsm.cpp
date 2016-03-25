////////////////////////////////////////////////////////////////////////////////
// Name:      vifsm.cpp
// Purpose:   Implementation of class wxExViFSM
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/extension/vifsm.h>
#include <wx/extension/managedframe.h>
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
    wxExViFSMEntry(wxExVi::STATE, ACTION, wxExVi::NEXT, PROCESS)   \

#define NAVIGATE(SCOPE, DIRECTION)                                 \
  m_vi->GetSTC()->SCOPE##DIRECTION();                              \
  
wxExViFSM::wxExViFSM(wxExVi* vi, 
  std::function<void(const std::string&)> insert,
  std::function<void(const std::string&)> normal)
  : m_vi(vi)
  , m_State(wxExVi::MODE_NORMAL)
  , m_InsertCommands {
    {'a', [&](){NAVIGATE(Char, Right);}},
    {'i', [&](){;}},
    {'o', [&](){
      NAVIGATE(Line, End);
      m_vi->GetSTC()->NewLine();}},
    {'A', [&](){NAVIGATE(Line, End);}},
    {'C', [&](){
      m_vi->GetSTC()->LineEndExtend();
      m_vi->Cut();}},
    {'I', [&](){NAVIGATE(Line, Home);}},
    {'O', [&](){
      NAVIGATE(Line, Home); 
      m_vi->GetSTC()->NewLine(); 
      NAVIGATE(Line, Up);}},
    {'R', [&](){m_vi->GetSTC()->SetOvertype(true);}}}
  , m_FSM {
      MAKE_ENTRY(MODE_NORMAL,      KEY_INSERT,      MODE_INSERT,      insert),
      MAKE_ENTRY(MODE_NORMAL,      KEY_VISUAL,      MODE_VISUAL,      nullptr),
      MAKE_ENTRY(MODE_NORMAL,      KEY_VISUAL_LINE, MODE_VISUAL_LINE, nullptr),
      MAKE_ENTRY(MODE_NORMAL,      KEY_VISUAL_RECT, MODE_VISUAL_RECT, nullptr),
      MAKE_ENTRY(MODE_INSERT,      KEY_ESCAPE,      MODE_NORMAL,      normal),
      MAKE_ENTRY(MODE_INSERT_RECT, KEY_ESCAPE,      MODE_VISUAL_RECT, nullptr),
      MAKE_ENTRY(MODE_VISUAL,      KEY_INSERT,      MODE_INSERT,      insert),
      MAKE_ENTRY(MODE_VISUAL,      KEY_ESCAPE,      MODE_NORMAL,      nullptr),
      MAKE_ENTRY(MODE_VISUAL,      KEY_VISUAL_LINE, MODE_VISUAL_LINE, nullptr),
      MAKE_ENTRY(MODE_VISUAL,      KEY_VISUAL_RECT, MODE_VISUAL_RECT, nullptr),
      MAKE_ENTRY(MODE_VISUAL_LINE, KEY_ESCAPE,      MODE_NORMAL,      nullptr),
      MAKE_ENTRY(MODE_VISUAL_LINE, KEY_VISUAL,      MODE_VISUAL,      nullptr),
      MAKE_ENTRY(MODE_VISUAL_LINE, KEY_VISUAL_RECT, MODE_VISUAL_RECT, nullptr),
      MAKE_ENTRY(MODE_VISUAL_RECT, KEY_INSERT,      MODE_INSERT_RECT, insert),
      MAKE_ENTRY(MODE_VISUAL_RECT, KEY_ESCAPE,      MODE_NORMAL,      nullptr),
      MAKE_ENTRY(MODE_VISUAL_RECT, KEY_VISUAL,      MODE_VISUAL,      nullptr),
      MAKE_ENTRY(MODE_VISUAL_RECT, KEY_VISUAL_LINE, MODE_VISUAL_LINE, nullptr)}
{
}

bool wxExViFSM::Transition(const std::string& command)
{
  int key = 0;
  
  switch (command.size())
  {
    case 0: return false;
    case 1:
      if (std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
        [command](auto const& e) {return e.first == command[0];}) 
        != m_InsertCommands.end())
      {
        key = KEY_INSERT;
      }
      else switch (command[0])
      {
        case 'K': key = KEY_VISUAL_RECT; break;
        case 'v': key = KEY_VISUAL; break;
        case 'V': key = KEY_VISUAL_LINE; break;
        case 27:  key = KEY_ESCAPE; break;
        default: return false;
      }
      break;
    default:
      if (command[0] == 'c')
      {
        key = KEY_INSERT;
      }
      else
      {
        return false;
      }
  }
  
  const auto it = std::find_if(m_FSM.begin(), m_FSM.end(), 
    [&](const auto & e) {return e.State() == m_State && e.Action() == key;});
  
  if (it == m_FSM.end())
  {
    wxBell();
    return false;
  }
  
  if (key == KEY_INSERT &&
    (m_vi->GetSTC()->GetReadOnly() || m_vi->GetSTC()->HexMode()))
  {
    return true;
  }

  m_State = it->Next(command);
  wxString mode;
  
  switch (m_State)
  {
    case wxExVi::MODE_INSERT:
      mode = "insert";
      {
      auto it = std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
        [command](auto const& e) {return e.first == command[0];});
      if (it != m_InsertCommands.end() && it->second != nullptr)
        it->second();
      }
      break;
      
    case wxExVi::MODE_INSERT_RECT:  mode = "insert rect"; break;
    case wxExVi::MODE_VISUAL:       mode = "visual";      break;
    case wxExVi::MODE_VISUAL_RECT:  mode = "visual rect"; break;
    
    case wxExVi::MODE_VISUAL_LINE:
      mode = "visual line";
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
  
  m_vi->GetFrame()->StatusText(mode, "PaneMode");

  return true;
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
