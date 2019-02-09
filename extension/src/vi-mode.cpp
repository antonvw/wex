////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.cpp
// Purpose:   Implementation of class wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <fsm.h>
#include <wex/vi-mode.h>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/managedframe.h>
#include <wex/stc.h>
#include <wex/vi.h>
#include <wex/vi-macros.h>
#include <wex/vi-macros-mode.h>

namespace wex
{
  /// This class holds the table containing the states.
  class vi_fsm
  {
  public:
    enum trigger_t
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
        // from state                   to state                       trigger      guard    action
      m_fsm.add_transitions({
        // insert
        {vi_mode::state_t::INSERT,      vi_mode::state_t::NORMAL,      ESCAPE,      nullptr, normal},
        {vi_mode::state_t::INSERT_RECT, vi_mode::state_t::VISUAL_RECT, ESCAPE,      nullptr, nullptr},
        // normal
        {vi_mode::state_t::NORMAL,      vi_mode::state_t::INSERT,      INSERT,      
           [&]{return m_writeable;}, [&]{insert_mode();}},
        {vi_mode::state_t::NORMAL,      vi_mode::state_t::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state_t::NORMAL,      vi_mode::state_t::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state_t::NORMAL,      vi_mode::state_t::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr},
        // visual
        {vi_mode::state_t::VISUAL,      vi_mode::state_t::NORMAL,      ESCAPE,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL,      vi_mode::state_t::INSERT,      INSERT,      
           [&]{return m_writeable;}, [&]{insert_mode();}},
        {vi_mode::state_t::VISUAL,      vi_mode::state_t::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL,      vi_mode::state_t::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state_t::VISUAL,      vi_mode::state_t::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr},
        // visual line
        {vi_mode::state_t::VISUAL_LINE, vi_mode::state_t::NORMAL,      ESCAPE,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL_LINE, vi_mode::state_t::VISUAL_LINE, INSERT,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL_LINE, vi_mode::state_t::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL_LINE, vi_mode::state_t::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state_t::VISUAL_LINE, vi_mode::state_t::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr},
        // visual rect
        {vi_mode::state_t::VISUAL_RECT, vi_mode::state_t::NORMAL,      ESCAPE,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL_RECT, vi_mode::state_t::INSERT_RECT, INSERT,      
           [&]{return m_writeable;}, [&]{insert_mode();}},
        {vi_mode::state_t::VISUAL_RECT, vi_mode::state_t::VISUAL,      VISUAL,      nullptr, nullptr},
        {vi_mode::state_t::VISUAL_RECT, vi_mode::state_t::VISUAL_LINE, VISUAL_LINE, nullptr, nullptr},
        {vi_mode::state_t::VISUAL_RECT, vi_mode::state_t::VISUAL_RECT, VISUAL_RECT, nullptr, nullptr}});

      m_fsm.add_debug_fn(verbose);
    };
    
    FSM::Fsm_Errors execute(const std::string& command, trigger_t trigger) {
      m_Command = command;
      m_writeable = !m_STC->GetReadOnly();
      return m_fsm.execute(trigger);};

    vi_mode::state_t get() const {return m_fsm.state();};

    void insert_mode() {
      if (m_Insert != nullptr)
      {
        m_Insert(m_Command);
      }};

    const std::string string() const {return string(get(), m_STC);};
     
    static const std::string string(vi_mode::state_t state, stc* stc) {
      switch (state)
      {
        case vi_mode::state_t::INSERT:      
          return stc != nullptr && stc->GetOvertype() ? "replace": "insert";
        case vi_mode::state_t::INSERT_RECT: 
          return stc != nullptr && stc->GetOvertype() ? "replace rect": "insert rect";
        case vi_mode::state_t::VISUAL:      return "visual";
        case vi_mode::state_t::VISUAL_LINE: return "visual line";
        case vi_mode::state_t::VISUAL_RECT: return "visual rect";
        default: return vi_macros::mode()->string();
      }};

    static const std::string string(trigger_t trigger) {
      switch (trigger)
      {
        case vi_fsm::INSERT: return "insert";
        case vi_fsm::VISUAL: return "visual";
        case vi_fsm::VISUAL_LINE: return "visual line";
        case vi_fsm::VISUAL_RECT: return "visual rect";
        case vi_fsm::ESCAPE: return "escape";
        default: return "unhandled";
      }};
    
    static void verbose(vi_mode::state_t from, vi_mode::state_t to, trigger_t trigger)
    {
      log::verbose(2) << 
        "vi mode trigger" << string(trigger) <<
        "state from" << string(from, nullptr) << 
        "to" << string(to, nullptr);
    };
  private:
    stc* m_STC;
    std::string m_Command;
    std::function<void(const std::string& command)> m_Insert;
    bool m_writeable;
    FSM::Fsm<vi_mode::state_t, vi_mode::state_t::NORMAL, trigger_t> m_fsm;
  };
};

#define NAVIGATE(SCOPE, DIRECTION)      \
  m_vi->get_stc()->SCOPE##DIRECTION();  \
  
wex::vi_mode::vi_mode(vi* vi, 
  std::function<void(const std::string&)> insert, 
  std::function<void()> normal)
  : m_vi(vi)
  , m_FSM(std::make_unique<vi_fsm>(vi->get_stc(), insert, normal))
  , m_InsertCommands {
    {'a', [&](){NAVIGATE(Char, Right);}},
    {'c', [&](){;}},
    {'i', [&](){;}},
    {'o', [&](){
      NAVIGATE(Line, End);
      m_vi->get_stc()->NewLine();}},
    {'A', [&](){NAVIGATE(Line, End);}},
    {'C', [&](){
      m_vi->get_stc()->LineEndExtend();
      m_vi->cut();}},
    {'I', [&](){NAVIGATE(Line, Home);}},
    {'O', [&](){
      NAVIGATE(Line, Home); 
      m_vi->get_stc()->NewLine(); 
      NAVIGATE(Line, Up);}},
    {'R', [&](){m_vi->get_stc()->SetOvertype(true);}}}
{
}

wex::vi_mode::~vi_mode()
{
}

wex::vi_mode::state_t wex::vi_mode::get() const
{
  return m_FSM->get();
}

const std::string wex::vi_mode::string() const
{
  return m_FSM->string();
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

  vi_fsm::trigger_t trigger = vi_fsm::INSERT;
  
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
  
  if (m_FSM->execute(command, trigger) != FSM::Fsm_Success)
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
      if (!m_vi->get_stc()->is_hexmode())
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
      if (m_vi->get_stc()->SelectionIsRectangle())
      {
        m_vi->get_stc()->Home();
        m_vi->get_stc()->LineDownExtend();
      }
      else if (m_vi->get_stc()->GetSelectedText().empty())
      {
        m_vi->get_stc()->Home();
        m_vi->get_stc()->LineDownExtend();
      }
      else
      {
        const auto selstart = m_vi->get_stc()->PositionFromLine(m_vi->get_stc()->LineFromPosition(m_vi->get_stc()->GetSelectionStart()));
        const auto selend = m_vi->get_stc()->PositionFromLine(m_vi->get_stc()->LineFromPosition(m_vi->get_stc()->GetSelectionEnd()) + 1);
        m_vi->get_stc()->SetSelection(selstart, selend);
        m_vi->get_stc()->HomeExtend();
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
