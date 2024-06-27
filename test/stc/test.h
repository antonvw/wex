////////////////////////////////////////////////////////////////////////////////
// Name:      test.h
// Purpose:   Declaration of classes for unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/stc/stc.h>
#include <wex/test/test.h>
#include <wex/ui/frame.h>

namespace wex
{
namespace test
{
class stc_frame : public frame
{
public:
  // Constructor.
  stc_frame()
    : frame()
  {
  }

  // Virtual interface.

  int stc_entry_dialog_show(bool modal = false) override
  {
    m_entry_invoked++;
    return wxID_CANCEL;
  };

  syntax::stc* stc_entry_dialog_component() override { return m_stc; }

  // Other methods.

  int entry_dialog_calls() const { return m_entry_invoked; };

  void entry_dialog_calls_reset() { m_entry_invoked = 0; };

  void make() { m_stc = new stc(); }

private:
  int       m_entry_invoked{0};
  wex::stc* m_stc = nullptr;
};

/// This class offers an stc_app, adding stc and frame to the wex::app.
class stc_app : public app
{
public:
  // Static methods

  static auto* frame() { return m_frame; }
  static auto* get_stc() { return m_stc; }

  // Virtual interface

  bool OnInit() override;

private:
  inline static stc_frame* m_frame = nullptr;
  inline static wex::stc*  m_stc   = nullptr;
};
}; // namespace test
}; // namespace wex

/// Returns the frame.
wex::test::stc_frame* frame();

/// Returns an stc.
wex::stc* get_stc();

/// Sends char event to window.
void event(wxWindow* win, char id);

/// Sends char events to window (each char is a separate event).
void event(wxWindow* win, const std::string& ids);

/// Sends key event to window.
void event(wxWindow* win, int id, wxEventType eventType = wxEVT_KEY_DOWN);
