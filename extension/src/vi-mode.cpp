////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.cpp
// Purpose:   Implementation of class wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <wex/vi-mode.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/vi.h>
#include <wex/vi-macros.h>
#include <wex/vi-macros-mode.h>
#include <easylogging++.h>
#include "fsm.h"

namespace wex
{
  /// This class holds the table containing the states.
  class vi_fsm
  {
  public:
    enum trigger
    {
      INSERT,
      VISUAL,
      VISUAL_LINE,
      VISUAL_RECT,
      ESCAPE
    };

    vi_fsm(stc* stc, 
      std::function<void(const std::string& command)> insert, 
      std::function<void()> normal)
      : m_STC(stc)
      , m_Insert(insert)
    {
        // from state                 to state                     trigger       guard    action
      m_fsm.add_transitions({
        // insert
        {vi_mode::state::INSERT,      vi_mode::state::NORMAL,      ESCAPE,      nullptr, normal},
        {vi_mode::state::INSERT_RECT, vi_mode::state::VISUAL_RECT, ESCAPE,      nullptr, nullptr},
        // normal
        {vi_mode::state::NORMAL,      vi_mode::state::INSERT,      INSERT,      
           [&]{return m_writeable;}, [&]{InsertMode();}},
        {vi_mode::state::NORMAL,      vi_mode::state::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state::NORMAL,      vi_mode::state::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state::NORMAL,      vi_mode::state::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr},
        // visual
        {vi_mode::state::VISUAL,      vi_mode::state::NORMAL,      ESCAPE,      nullptr, nullptr},
        {vi_mode::state::VISUAL,      vi_mode::state::INSERT,      INSERT,      
           [&]{return m_writeable;}, [&]{InsertMode();}},
        {vi_mode::state::VISUAL,      vi_mode::state::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state::VISUAL,      vi_mode::state::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state::VISUAL,      vi_mode::state::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr},
        // visual line
        {vi_mode::state::VISUAL_LINE, vi_mode::state::NORMAL,      ESCAPE,      nullptr, nullptr},
        {vi_mode::state::VISUAL_LINE, vi_mode::state::VISUAL_LINE, INSERT,      nullptr, nullptr},
        {vi_mode::state::VISUAL_LINE, vi_mode::state::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state::VISUAL_LINE, vi_mode::state::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state::VISUAL_LINE, vi_mode::state::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr},
        // visual rect
        {vi_mode::state::VISUAL_RECT, vi_mode::state::NORMAL,      ESCAPE,      nullptr, nullptr},
        {vi_mode::state::VISUAL_RECT, vi_mode::state::INSERT_RECT, INSERT,      
           [&]{return m_writeable;}, [&]{InsertMode();}},
        {vi_mode::state::VISUAL_RECT, vi_mode::state::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state::VISUAL_RECT, vi_mode::state::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state::VISUAL_RECT, vi_mode::state::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr}});

      m_fsm.add_debug_fn(Verbose);
    };
    
    FSM::Fsm_Errors Execute(const std::string& command, trigger trigger) {
      m_Command = command;
      m_writeable = !m_STC->GetReadOnly();
      return m_fsm.execute(trigger);};

    vi_mode::state Get() const {return m_fsm.state();};

    void InsertMode() {
      if (m_Insert != nullptr)
      {
        m_Insert(m_Command);
      }};

    const std::string String() const {return String(Get(), m_STC);};
     
    static const std::string String(vi_mode::state state, stc* stc) {
      switch (state)
      {
        case vi_mode::state::INSERT:      
          return stc != nullptr && stc->GetOvertype() ? "replace": "insert";
        case vi_mode::state::INSERT_RECT: 
          return stc != nullptr && stc->GetOvertype() ? "replace rect": "insert rect";
        case vi_mode::state::VISUAL:      return "visual";
        case vi_mode::state::VISUAL_LINE: return "visual line";
        case vi_mode::state::VISUAL_RECT: return "visual rect";
        default: return vi_macros::mode()->string();
      }};

    static const std::string String(trigger trigger) {
      switch (trigger)
      {
        case vi_fsm::INSERT: return "insert";
        case vi_fsm::VISUAL: return "visual";
        case vi_fsm::VISUAL_LINE: return "visual line";
        case vi_fsm::VISUAL_RECT: return "visual rect";
        case vi_fsm::ESCAPE: return "escape";
        default: return "unhandled";
      }};
    
    static void Verbose(vi_mode::state from, vi_mode::state to, trigger trigger)
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
    FSM::Fsm<vi_mode::state, vi_mode::state::NORMAL, trigger> m_fsm;
  };
};

#define NAVIGATE(SCOPE, DIRECTION)        \
  m_vi->stc()->SCOPE##DIRECTION();        \
  
wex::vi_mode::vi_mode(vi* vi, 
  std::function<void(const std::string&)> insert, 
  std::function<void()> normal)
  : m_vi(vi)
  , m_FSM(std::make_unique<vi_fsm>(vi->stc(), insert, normal))
  , m_InsertCommands {
    {'a', [&](){NAVIGATE(Char, Right);}},
    {'c', [&](){;}},
    {'i', [&](){;}},
    {'o', [&](){
      NAVIGATE(Line, End);
      m_vi->stc()->NewLine();}},
    {'A', [&](){NAVIGATE(Line, End);}},
    {'C', [&](){
      m_vi->stc()->LineEndExtend();
      m_vi->cut();}},
    {'I', [&](){NAVIGATE(Line, Home);}},
    {'O', [&](){
      NAVIGATE(Line, Home); 
      m_vi->stc()->NewLine(); 
      NAVIGATE(Line, Up);}},
    {'R', [&](){m_vi->stc()->SetOvertype(true);}}}
{
}

wex::vi_mode::~vi_mode()
{
}

wex::vi_mode::state wex::vi_mode::get() const
{
  return m_FSM->Get();
}

const std::string wex::vi_mode::string() const
{
  return m_FSM->String();
}
  
bool wex::vi_mode::transition(std::string& command)
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

  vi_fsm::trigger trigger = vi_fsm::INSERT;
  
  if (std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
    [command](auto const& e) {return e.first == command[0];}) 
    != m_InsertCommands.end())
  {
    trigger = vi_fsm::INSERT;
  }
  else switch (command[0])
  {
    case 'K': trigger = vi_fsm::VISUAL_RECT; break;
    case 'v': trigger = vi_fsm::VISUAL; break;
    case 'V': trigger = vi_fsm::VISUAL_LINE; break;
    case 27:  trigger = vi_fsm::ESCAPE; break;
    default: return false;
  }
  
  if (m_FSM->Execute(command, trigger) != FSM::Fsm_Success)
  {
    if (config(_("Error bells")).get(true))
    {
      wxBell();
    }

    return false;
  }
  
  switch (get())
  {
    case INSERT:
      if (!m_vi->stc()->is_hexmode())
      {
        if (const auto& it = std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
          [command](auto const& e) {return e.first == command[0];});
          it != m_InsertCommands.end() && it->second != nullptr)
          {
            it->second();
            m_vi->append_insert_command(command.substr(0, 1));
          }
      }
      break;
      
    case VISUAL_LINE:
      if (m_vi->stc()->SelectionIsRectangle())
      {
        m_vi->stc()->Home();
        m_vi->stc()->LineDownExtend();
      }
      else if (m_vi->stc()->GetSelectedText().empty())
      {
        m_vi->stc()->Home();
        m_vi->stc()->LineDownExtend();
      }
      else
      {
        const int selstart = m_vi->stc()->PositionFromLine(m_vi->stc()->LineFromPosition(m_vi->stc()->GetSelectionStart()));
        const int selend = m_vi->stc()->PositionFromLine(m_vi->stc()->LineFromPosition(m_vi->stc()->GetSelectionEnd()) + 1);
        m_vi->stc()->SetSelection(selstart, selend);
        m_vi->stc()->HomeExtend();
      }
      break;
    
    default: break;
  }

  ((statusbar *)m_vi->frame()->GetStatusBar())->show_field(
    "PaneMode", 
    (!normal() || vi_macros::mode()->is_recording()) && 
       config(_("Show mode")).get(false));

  frame::statustext(string(), "PaneMode");

  command = command.substr(1);

  return true;
}
