////////////////////////////////////////////////////////////////////////////////
// Name:      function-repeat.h
// Purpose:   Declaration of class wex::function_repeat
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/reflection.h>
#include <wx/timer.h>

namespace wex
{
/// Offers a class that allows repeating a function on
/// an event handler (window).
class function_repeat
{
public:
  /// Callback for event action.
  typedef std::function<void(wxTimerEvent&)> repeat_t;

  /// Constructor.
  function_repeat(
    const std::string& name, ///< name for the repeater
    wxEvtHandler*      evt,  ///< where event will be handled
    repeat_t           t     ///< the function to be repeated
  );

  /// Destructor.
  ~function_repeat();

  /// Starts or stops repeating.
  /// Returns true if action is successful.
  bool activate(bool start = true);

  /// Returns name.
  const auto& name() const { return m_name; };

private:
  enum class action_t;

  bool action(action_t);

  wxEvtHandler* m_handler;
  wxTimer*      m_timer{nullptr};

  wxWindowID m_timer_id;

  repeat_t m_f;

  reflection m_reflect;

  const std::string m_name;
};
}; // namespace wex
