////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.h
// Purpose:   Declaration of wex::textctrl class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/data/window.h>

class wxControl;

namespace wex
{
namespace factory
{
class stc;
};

class frame;
class textctrl_imp;

/// Offers a text ctrl related to a factory::stc object.
class textctrl
{
public:
  /// Constructor. Creates empty control.
  explicit textctrl(frame* frame, wxControl* prefix, const data::window& data);

  /// Constructor. Skips prefix.
  explicit textctrl(
    frame*              frame,
    const std::string&  value = std::string(),
    const data::window& data  = data::window());

  /// Returns the control window for the component.
  wxControl* control();

  /// Returns frame.
  auto* get_frame() { return m_frame; }

  /// Get string value.
  const std::string get_text() const;

  /// Destroys the implementation.
  void on_exit();

  /// Selects all text.
  void select_all() const;

  /// Sets stc component using string command.
  /// Returns false if command not supported.
  bool set_stc(wex::factory::stc* stc, const std::string& command);

  /// Sets stc component using char command.
  /// Returns false if command not supported.
  bool set_stc(wex::factory::stc* stc, char command);

  /// Sets text.
  void set_text(const std::string& text);

  /// Returns stc component.
  auto* stc() { return m_stc; }

private:
  wex::factory::stc* m_stc{nullptr};
  frame*             m_frame;
  textctrl_imp*      m_imp;
};
}; // namespace wex
