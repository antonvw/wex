////////////////////////////////////////////////////////////////////////////////
// Name:      vi-mode.cpp
// Purpose:   Implementation of class wxExViMode
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

enum class Triggers
{
  INSERT,
  VISUAL,
  VISUAL_LINE,
  VISUAL_RECT,
  ESCAPE
};

/// This class holds the table containing the states.
class wxExViFSM
{
public:
  wxExViFSM(wxExSTC* stc, 
    std::function<void(const std::string& command)> insert, 
    std::function<void()> normal)
    : m_STC(stc)
    , m_Insert(insert)
  {
      // from state              to state                  triggers               guard    action
    m_fsm.add_transitions({
      // insert
      {wxExViModes::INSERT,      wxExViModes::NORMAL,      Triggers::ESCAPE,      nullptr, normal},
      {wxExViModes::INSERT_RECT, wxExViModes::VISUAL_RECT, Triggers::ESCAPE,      nullptr, nullptr},
      // normal
      {wxExViModes::NORMAL,      wxExViModes::INSERT,      Triggers::INSERT,      
         [&]{return m_writeable;}, [&]{InsertMode();}},
      {wxExViModes::NORMAL,      wxExViModes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
      {wxExViModes::NORMAL,      wxExViModes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
      {wxExViModes::NORMAL,      wxExViModes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr},
      // visual
      {wxExViModes::VISUAL,      wxExViModes::NORMAL,      Triggers::ESCAPE,      nullptr, nullptr},
      {wxExViModes::VISUAL,      wxExViModes::INSERT,      Triggers::INSERT,      
         [&]{return m_writeable;}, [&]{InsertMode();}},
      {wxExViModes::VISUAL,      wxExViModes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
      {wxExViModes::VISUAL,      wxExViModes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
      {wxExViModes::VISUAL,      wxExViModes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr},
      // visual line
      {wxExViModes::VISUAL_LINE, wxExViModes::NORMAL,      Triggers::ESCAPE,      nullptr, nullptr},
      {wxExViModes::VISUAL_LINE, wxExViModes::VISUAL_LINE, Triggers::INSERT,      nullptr, nullptr},
      {wxExViModes::VISUAL_LINE, wxExViModes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
      {wxExViModes::VISUAL_LINE, wxExViModes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
      {wxExViModes::VISUAL_LINE, wxExViModes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr},
      // visual rect
      {wxExViModes::VISUAL_RECT, wxExViModes::NORMAL,      Triggers::ESCAPE,      nullptr, nullptr},
      {wxExViModes::VISUAL_RECT, wxExViModes::INSERT_RECT, Triggers::INSERT,      
         [&]{return m_writeable;}, [&]{InsertMode();}},
      {wxExViModes::VISUAL_RECT, wxExViModes::VISUAL,      Triggers::VISUAL,      nullptr, nullptr},
      {wxExViModes::VISUAL_RECT, wxExViModes::VISUAL_LINE, Triggers::VISUAL_LINE, nullptr, nullptr},
      {wxExViModes::VISUAL_RECT, wxExViModes::VISUAL_RECT, Triggers::VISUAL_RECT, nullptr, nullptr}});

    m_fsm.add_debug_fn(Verbose);
  };
  
  FSM::Fsm_Errors Execute(const std::string& command, Triggers trigger) {
    m_Command = command;
    m_writeable = !m_STC->GetReadOnly();
    return m_fsm.execute(trigger);};

  wxExViModes Get() const {return m_fsm.state();};

  void InsertMode() {
    if (m_Insert != nullptr)
    {
      m_Insert(m_Command);
    }};

  const std::string String() const {return String(Get(), m_STC);};
   
  static const std::string String(wxExViModes state, wxExSTC* stc) {
    switch (state)
    {
      case wxExViModes::INSERT:      
        return stc != nullptr && stc->GetOvertype() ? "replace": "insert";
      case wxExViModes::INSERT_RECT: 
        return stc != nullptr && stc->GetOvertype() ? "replace rect": "insert rect";
      case wxExViModes::VISUAL:      return "visual";
      case wxExViModes::VISUAL_LINE: return "visual line";
      case wxExViModes::VISUAL_RECT: return "visual rect";
      default: return wxExViMacros::Mode()->String();
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
  
  static void Verbose(wxExViModes from, wxExViModes to, Triggers trigger)
  {
    VLOG(2) << 
      "vi mode trigger " << String(trigger) <<
      " state from " << String(from, nullptr) << 
      " to " << String(to, nullptr);
  };
private:
  wxExSTC* m_STC;
  std::string m_Command;
  std::function<void(const std::string& command)> m_Insert;
  bool m_writeable;
  FSM::Fsm<wxExViModes, wxExViModes::NORMAL, Triggers> m_fsm;
};

#define NAVIGATE(SCOPE, DIRECTION)                   \
  m_vi->GetSTC()->SCOPE##DIRECTION();                \
  
wxExViMode::wxExViMode(wxExVi* vi, 
  std::function<void(const std::string&)> insert, 
  std::function<void()> normal)
  : m_vi(vi)
  , m_FSM(std::make_unique<wxExViFSM>(vi->GetSTC(), insert, normal))
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

wxExViMode::~wxExViMode()
{
}

wxExViModes wxExViMode::Get() const
{
  return m_FSM->Get();
}

const std::string wxExViMode::String() const
{
  return m_FSM->String();
}
  
bool wxExViMode::Transition(const std::string& command)
{
  Triggers trigger = Triggers::INSERT;
  
  switch (command.size())
  {
    case 0: return false;
    case 1:
      if (command[0] == 'c' && m_vi->GetSTC()->GetSelectedText().empty())
      {
        return false;
      }
      else if (std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
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
      break;
    default:
      if (command[0] != 'c')
      {
        return false;
      }
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
    case wxExViModes::INSERT:
      if (!m_vi->GetSTC()->HexMode())
      {
        const auto& it = std::find_if(m_InsertCommands.begin(), m_InsertCommands.end(), 
          [command](auto const& e) {return e.first == command[0];});
        if (it != m_InsertCommands.end() && it->second != nullptr)
          it->second();
      }
      break;
      
    case wxExViModes::VISUAL_LINE:
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

  ((wxExStatusBar *)m_vi->GetFrame()->GetStatusBar())->ShowField(
    "PaneMode", 
    (!Normal() || wxExViMacros::Mode()->IsRecording()) && 
       wxConfigBase::Get()->ReadBool(_("Show mode"), false));

  wxExFrame::StatusText(String(), "PaneMode");

  return true;
}
