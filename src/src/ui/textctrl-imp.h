////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp.h
// Purpose:   Declaration of wex::textctrl_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/textctrl-input.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

namespace wex
{
  class textctrl;

  class textctrl_imp : public wxTextCtrl
  {
  public:
    textctrl_imp(textctrl* tc, wxControl* prefix, const window_data& data);

    textctrl_imp(
      textctrl*          tc,
      const std::string& value = std::string(),
      const window_data& data  = window_data());

    const std::string get_text() const;
    bool              handle(const std::string& command);
    bool              handle(char command);
    void              set_text(const std::string& text);

    void SelectAll() override;

  private:
    void            bind();
    void            cut();
    bool            input_mode_finish() const;
    textctrl_input& TCI();

    wxControl* m_prefix{nullptr};
    textctrl*  m_tc;
    wxTimer    m_timer;

    char m_input{0};

    bool m_control_r{false}, m_mode_visual{false}, m_user_input{false};

    textctrl_input m_calcs{ex_command::type_t::CALC},
      m_commands{ex_command::type_t::COMMAND},
      m_execs{ex_command::type_t::EXEC},
      m_find_margins{ex_command::type_t::FIND_MARGIN};

    ex_command m_command;
  };
}; // namespace wex
