////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.h
// Purpose:   Declaration of wex::textctrl class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/window-data.h>

class wxControl;

namespace wex
{
  class ex;
  class managed_frame;
  class textctrl_imp;

  /// Offers a text ctrl related to a ex object.
  class textctrl
  {
  public:
    /// Constructor. Creates empty control.
    textctrl(managed_frame* frame, wxControl* prefix, const data::window& data);

    /// Constructor. Skips prefix.
    textctrl(
      managed_frame*     frame,
      const std::string& value = std::string(),
      const data::window& data  = data::window());

    /// Destructor.
    ~textctrl();

    /// Returns the control window for the component.
    wxControl* control();

    /// Returns ex component.
    wex::ex* ex() { return m_ex; };

    /// Returns frame.
    managed_frame* frame() { return m_frame; };

    /// Get string value.
    const std::string get_text() const;

    /// Selects all text.
    void select_all() const;

    /// Sets ex component using string command.
    /// Returns false if command not supported.
    bool set_ex(wex::ex* ex, const std::string& command);

    /// Sets ex component using char command.
    /// Returns false if command not supported.
    bool set_ex(wex::ex* ex, char command);

    /// Sets text.
    void set_text(const std::string& text);

  private:
    wex::ex*       m_ex{nullptr};
    managed_frame* m_frame;
    textctrl_imp*  m_imp;
  };
}; // namespace wex
