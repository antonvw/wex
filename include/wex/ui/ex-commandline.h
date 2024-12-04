////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline.h
// Purpose:   Declaration of wex::ex_commandline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/window.h>

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

  // Virtual interface

  /// Finds current text on the component, default the stc
  /// component is used.
  virtual bool find();

  /// Process on enter key, default ex action, when
  /// returning true skips defaults.
  virtual bool find_on_enter() { return false; };

  // Other methods

  /// Returns the stc control window for the component,
  /// the commandline itself is just another stc, just as
  /// the one it is related to (through set_stc and stc).
  syntax::stc* control();

  /// Returns frame.
  auto* get_frame() { return m_frame; }

  /// Get string value.
  const std::string get_text() const;

  /// Handles keydown event.
  void on_key_down(wxKeyEvent& event);

  /// Selects all text.
  void select_all() const;

  /// Sets stc component.
  void set_stc(wex::syntax::stc* stc) { m_stc = stc; }

  /// Sets stc component and handles string command.
  /// Returns false if command not supported.
  bool set_stc(wex::syntax::stc* stc, const std::string& command);

  /// Sets stc component and handles char command to
  /// enter the input mode (a or c or i).
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
