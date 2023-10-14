////////////////////////////////////////////////////////////////////////////////
// Name:      function-repeat.cpp
// Purpose:   Implementation of class wex::function_repeat
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/function-repeat.h>
#include <wex/core/log.h>
#include <wx/window.h>

/// Supported actions.
enum class wex::function_repeat::action_t
{
  START, ///< starts the action
  STOP   ///< stops the action
};

wex::function_repeat::function_repeat(
  const std::string& name,
  wxEvtHandler*      evt,
  repeat_t           f)
  : m_name(name)
  , m_handler(evt)
  , m_f(f)
{
}

wex::function_repeat::~function_repeat()
{
  m_timer->Stop();

  delete m_timer;
}

bool wex::function_repeat::action(action_t a)
{
  switch (a)
  {
    case action_t::START:
      if (
        !config("AllowSync").get(true) ||
        (m_timer != nullptr && m_timer->IsRunning()))
      {
        return false;
      }

      m_timer_id = wxWindowBase::NewControlId();
      m_timer    = new wxTimer(m_handler, m_timer_id);

      if (!m_timer->Start(config("Repeater").get(1000)))
      {
        return false;
      }

      log::trace("repeat " + m_name)
        << "id:" << m_timer_id << "interval:" << m_timer->GetInterval() << "\n";

      m_handler->Bind(wxEVT_TIMER, m_f, m_timer_id);
      break;

    case action_t::STOP:
      if (m_timer == nullptr || !m_timer->IsRunning())
      {
        return false;
      }

      m_timer->Stop();
      m_handler->Unbind(wxEVT_TIMER, m_f, m_timer_id);
      break;
  }

  return true;
}

bool wex::function_repeat::activate(bool start)
{
  return action(start ? action_t::START : action_t::STOP);
}
