////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline.h
// Purpose:   Declaration of wex::ex_commandline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/window.h>

class wxControl;

namespace wex
{
namespace syntax
{
class stc;
};

class frame;
class ex_commandline_imp;

/// Offers a ex commandline control related to a syntax::stc object,
/// allowing you to enter ex commands for that stc object in the
/// commandline.
class ex_commandline
{
public:
  /// Constructor. Creates empty control.
  explicit ex_commandline(
    frame*              frame,
    wxControl*          prefix,
    const data::window& data);

  /// Constructor. Skips prefix.
  explicit ex_commandline(
    frame*              frame,
    const std::string&  value = std::string(),
    const data::window& data  = data::window());

  /// Returns the stc control window for the component.
  syntax::stc* control();

  /// Returns frame.
  auto* get_frame() { return m_frame; }

  /// Get string value.
  const std::string get_text() const;

  /// Destroys the implementation.
  void on_exit();

  /// Handles keydown event.
  void on_key_down(wxKeyEvent& event);

  /// Selects all text.
  void select_all() const;

  /// Sets stc component.
  void set_stc(wex::syntax::stc* stc) { m_stc = stc; }

  /// Sets stc component and handles string command.
  /// Returns false if command not supported.
  bool set_stc(wex::syntax::stc* stc, const std::string& command);

  /// Sets stc component and handles char command.
  /// Returns false if command not supported.
  bool set_stc(wex::syntax::stc* stc, char command);

  /// Sets text.
  void set_text(const std::string& text);

  /// Returns stc component.
  auto* stc() { return m_stc; }

private:
  wex::syntax::stc*   m_stc{nullptr};
  frame*              m_frame;
  ex_commandline_imp* m_imp;
};
}; // namespace wex
