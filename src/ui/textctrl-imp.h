////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp.h
// Purpose:   Declaration of wex::textctrl_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/ex-command.h>
#include <wx/textctrl.h>
#include <wx/timer.h>

namespace wex
{
class textctrl;
class textctrl_input;

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

  /// other methods

  /// Returns text.
  const std::string get_text();

  /// Handles string command.
  bool handle(const std::string& command);

  /// Handles chr command.
  bool handle(char command);

  /// Sets text.
  void set_text(const std::string& text);

  /// Virtual interface

  bool Destroy() override;
  void SelectAll() override;

private:
  void bind();
  void cut();
  bool input_mode_finish() const;
  bool is_ex_mode() const;

  void process_char(wxKeyEvent& event);
  void process_key_down(wxKeyEvent& event);
  void process_key_down_page(wxKeyEvent& event);
  void process_text(wxCommandEvent& event);
  void process_text_enter(wxCommandEvent& event);
  bool process_text_enter_prep(wxCommandEvent& event);
  void process_text_paste(wxCommandEvent& event);

  textctrl_input* tci();

  const int m_id_register;

  wxControl* m_prefix{nullptr};
  textctrl*  m_tc;
  wxTimer    m_timer;

  char m_input{0};

  bool m_all_selected{false}, m_control_r{false}, m_mode_visual{false},
    m_user_input{false};

  textctrl_input *m_calcs{nullptr}, *m_commands{nullptr},
    *m_commands_ex{nullptr}, *m_escapes{nullptr}, *m_find_margins{nullptr};

  ex_command m_command;
};
}; // namespace wex
