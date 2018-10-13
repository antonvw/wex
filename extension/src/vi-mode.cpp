////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.cpp
// Purpose:   Implementation of class wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wx/config.h>
#include <wx/extension/vi-mode.h>
#include <wx/extension/log.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>
#include <wx/extension/vi-macros.h>
#include <wx/extension/vi-macros-mode.h>
#include <easylogging++.h>
#include "fsm.h"

namespace wex
{
  enum class Triggers
  {
    INSERT,
    VISUAL,
    VISUAL_LINE,
    VISUAL_RECT,
    ESCAPE
  };

  /// This class holds the table containing the states.
  class vifsm
  {
  public:
    vifsm(stc* stc, 
      std::function<void(const std::string& command)> insert, 
      std::function<void()> normal)
      : m_STC(stc)
      , m_Insert(insert)
    {
        // from state              to state                  triggers               guard    action
      m_fsm.add_transitions({
        // insert
        {vi_modes::INSERT,      vi_modes::NORMAL,      Triggers::ESCAPE,      nullptr, normal},
        {vi_modes::INSERT_RECT, vi_modes::VISUAL_RECT, Triggers::ESCAPE,      nullptr, nullptr},
        // normal
        {vi_modes::NORMAL,      vi_modes::INSERT,      Triggers::INSERT,      
           [&]{return m_writeable;}, [&]{InsertMode();}},
        {vi_modes::NORMAL,      vi_modes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
        {vi_modes::NORMAL,      vi_modes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
        {vi_modes::NORMAL,      vi_modes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr},
        // visual
        {vi_modes::VISUAL,      vi_modes::NORMAL,      Triggers::ESCAPE,      nullptr, nullptr},
        {vi_modes::VISUAL,      vi_modes::INSERT,      Triggers::INSERT,      
           [&]{return m_writeable;}, [&]{InsertMode();}},
        {vi_modes::VISUAL,      vi_modes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
        {vi_modes::VISUAL,      vi_modes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
        {vi_modes::VISUAL,      vi_modes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr},
        // visual line
        {vi_modes::VISUAL_LINE, vi_modes::NORMAL,      Triggers::ESCAPE,      nullptr, nullptr},
        {vi_modes::VISUAL_LINE, vi_modes::VISUAL_LINE, Triggers::INSERT,      nullptr, nullptr},
        {vi_modes::VISUAL_LINE, vi_modes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
        {vi_modes::VISUAL_LINE, vi_modes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
        {vi_modes::VISUAL_LINE, vi_modes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr},
        // visual rect
        {vi_modes::VISUAL_RECT, vi_modes::NORMAL,      Triggers::ESCAPE,      nullptr, nullptr},
        {vi_modes::VISUAL_RECT, vi_modes::INSERT_RECT, Triggers::INSERT,      
           [&]{return m_writeable;}, [&]{InsertMode();}},
        {vi_modes::VISUAL_RECT, vi_modes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
        {vi_modes::VISUAL_RECT, vi_modes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
        {vi_modes::VISUAL_RECT, vi_modes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr}});

      m_fsm.add_debug_fn(Verbose);
    };
    
    FSM::Fsm_Errors Execute(const std::string& command, Triggers trigger) {
      m_Command = command;
      m_writeable = !m_STC->GetReadOnly();
      return m_fsm.execute(trigger);};

    vi_modes Get() const {return m_fsm.state();};

    void InsertMode() {
      if (m_Insert != nullptr)
      {
        m_Insert(m_Command);
      }};

    const std::string String() const {return String(Get(), m_STC);};
     
    static const std::string String(vi_modes state, stc* stc) {
      switch (state)
      {
        case vi_modes::INSERT:      
          return stc != nullptr && stc->GetOvertype() ? "replace": "insert";
        case vi_modes::INSERT_RECT: 
          return stc != nullptr && stc->GetOvertype() ? "replace rect": "insert rect";
        case vi_modes::VISUAL:      return "visual";
        case vi_modes::VISUAL_LINE: return "visual line";
        case vi_modes::VISUAL_RECT: return "visual rect";
        default: return vi_macros::Mode()->String();
      }};

    static const std::string String(Triggers trigger) {
      switch (trigger)
      {
        case Triggers::INSERT: return "insert";
        case Triggers::VISUAL: return "visual";
        case Triggers::VISUAL_LINE: return "visual line";
        case Triggers::VISUAL_RECT: return "visual rect";
        case Triggers::ESCAPE: return "escape";
        default: return "unhandled";
      }};
    
    static void Verbose(vi_modes from, vi_modes to, Triggers trigger)
    {
      VLOG(2) << 
        "vi mode trigger " << String(trigger) <<
        " state from " << String(from, nullptr) << 
        " to " << String(to, nullptr);
    };
  private:
    stc* m_STC;
    std::string m_Command;
    std::function<void(const std::string& command)> m_Insert;
    bool m_writeable;
    FSM::Fsm<vi_modes, vi_modes::NORMAL, Triggers> m_fsm;
  };
};

#define NAVIGATE(SCOPE, DIRECTION)                   \
  m_vi->GetSTC()->SCOPE##DIRECTION();                \
  
wex::vi_mode::vi_mode(vi* vi, 
  std::function<void(const std::string&)> insert, 
  std::function<void()> normal)
  : m_vi(vi)
  , m_FSM(std::make_unique<vifsm>(vi->GetSTC(), insert, normal))
  , m_InsertCommands {
    {'a', [&](){NAVIGATE(Char, Right);}},
    {'c', [&](){;}},
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
{
}

wex::vi_mode::~vi_mode()
{
}

wex::vi_modes wex::vi_mode::Get() const
{
  return m_FSM->Get();
}

const std::string wex::vi_mode::String() const
{
  return m_FSM->String();
}
  
bool wex::vi_mode::Transition(std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  else if (command[0] == 'c')
  {
    if (command.size() == 1)
    {
       return false;
    }
    else
    {
      if (command.size() == 2 && (command[1] == 'f' || command[1] == 'F'))
      {
        return false;
      }
    }
  }

  Triggers trigger = Triggers::INSERT;
  
  if (std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
    [command](auto const& e) {return e.first == command[0];}) 
    != m_InsertCommands.end())
  {
    trigger = Triggers::INSERT;
  }
  else switch (command[0])
  {
    case 'K': trigger = Triggers::VISUAL_RECT; break;
    case 'v': trigger = Triggers::VISUAL; break;
    case 'V': trigger = Triggers::VISUAL_LINE; break;
    case 27:  trigger = Triggers::ESCAPE; break;
    default: return false;
  }
  
  if (m_FSM->Execute(command, trigger) != FSM::Fsm_Success)
  {
    if (wxConfigBase::Get()->ReadLong(_("Error bells"), 1))
    {
      wxBell();
    }

    return false;
  }
  
  switch (Get())
  {
    case vi_modes::INSERT:
      if (!m_vi->GetSTC()->HexMode())
      {
        if (const auto& it = std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
          [command](auto const& e) {return e.first == command[0];});
          it != m_InsertCommands.end() && it->second != nullptr)
          {
            it->second();
            m_vi->AppendInsertCommand(command.substr(0, 1));
          }
      }
      break;
      
    case vi_modes::VISUAL_LINE:
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
    
    default: break;
  }

  ((statusbar *)m_vi->GetFrame()->GetStatusBar())->ShowField(
    "PaneMode", 
    (!Normal() || vi_macros::Mode()->IsRecording()) && 
       wxConfigBase::Get()->ReadBool(_("Show mode"), false));

  frame::StatusText(String(), "PaneMode");

  command = command.substr(1);

  return true;
}
