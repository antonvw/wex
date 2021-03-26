////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp.h
// Purpose:   Declaration of wex::textctrl_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/textctrl-input.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

namespace wex
{
  class textctrl;

  /// Offers textctrl implementation.
  class textctrl_imp : public wxTextCtrl
  {
  public:
    /// Constructor. Creates empty control.
    textctrl_imp(textctrl* tc, wxControl* prefix, const data::window& data);

    /// Constructor. Skips prefix.
    textctrl_imp(
      textctrl*           tc,
      const std::string&  value = std::string(),
      const data::window& data  = data::window());

    /// virtual interface

    /// Selects all.
    void SelectAll() override;

    /// other methods

    /// Returns text.
    const std::string get_text() const;

    /// Handles string command.
    bool handle(const std::string& command);

    /// Handles chr command.
    bool handle(char command);

    /// Sets text.
    void set_text(const std::string& text);

  private:
    void bind();
    void cut();
    bool input_mode_finish() const;
    bool is_ex_mode() const;

    textctrl_input& TCI();

    const int m_id_register;

    wxControl* m_prefix{nullptr};
    textctrl*  m_tc;
    wxTimer    m_timer;

    char m_input{0};

    bool m_all_selected{false}, m_control_r{false}, m_mode_visual{false},
      m_user_input{false};

    textctrl_input m_calcs{ex_command::type_t::CALC},
      m_commands{ex_command::type_t::COMMAND},
      m_commands_ex{ex_command::type_t::COMMAND_EX},
      m_escapes{ex_command::type_t::ESCAPE},
      m_find_margins{ex_command::type_t::FIND_MARGIN};

    ex_command m_command;
  };
}; // namespace wex
