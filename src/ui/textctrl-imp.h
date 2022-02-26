////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp.h
// Purpose:   Declaration of wex::textctrl_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
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
  explicit textctrl_imp(
    textctrl*           tc,
    wxControl*          prefix,
    const data::window& data);

  /// Constructor. Skips prefix.
  explicit textctrl_imp(
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
  void bind_timer();
  void cut();
  bool handle_type(const std::string& command, const std::string& range);
  bool input_mode_finish() const;
  bool is_ex_mode() const;

  void on_char(wxKeyEvent& event);
  void on_char_others(wxKeyEvent& event);
  void on_key_down(wxKeyEvent& event);
  void on_key_down_page(wxKeyEvent& event);
  void on_text(wxCommandEvent& event);
  void on_text_enter(wxCommandEvent& event);
  void on_text_enter_do();
  bool on_text_enter_prep();
  void on_text_paste(wxCommandEvent& event);

  void set_prefix();

  textctrl_input* tci();

  const int m_id_register;

  wxControl* m_prefix{nullptr};
  textctrl*  m_tc;
  wxTimer    m_timer;

  char m_input{0};

  bool m_all_selected{false}, m_control_r{false}, m_mode_visual{false},
    m_user_input{false};

  std::vector<textctrl_input*> m_tcis;

  ex_command m_command;
};
}; // namespace wex
