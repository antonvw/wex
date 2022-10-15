////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stc/stc.h>
#include <wex/ui/frame.h>

#include "../test.h"

namespace wex
{
namespace test
{
/// This class offers an stc_app, adding stc and frame to the wex::app.
class stc_app : public app
{
public:
  /// Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

  /// Virtual interface

  bool OnInit() override;

private:
  inline static wex::frame* m_frame = nullptr;
  inline static wex::stc*   m_stc   = nullptr;
};
}; // namespace test
}; // namespace wex

/// Returns the frame.
wex::frame* frame();

/// Returns an stc.
wex::stc* get_stc();

/// Sends char event to window.
void event(wxWindow* win, char id);

/// Sends char events to window (each char is a separate event).
void event(wxWindow* win, const std::string& ids);

/// Sends key event to window.
void event(wxWindow* win, int id, wxEventType eventType = wxEVT_KEY_DOWN);
