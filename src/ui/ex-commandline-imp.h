////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline-imp.h
// Purpose:   Declaration of wex::ex-commandline-imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/syntax/stc.h>

namespace wex
{
class ex_commandline;
class ex_commandline_input;

/// Offers the ex_commandline implementation.
/// It creates just another stc component that acts as the
/// ex commandline, providing lexer functionality for the commandline.
class ex_commandline_imp : public syntax::stc
{
public:
  /// Constructor. Creates empty control.
  explicit ex_commandline_imp(
    ex_commandline*     cl,
    wxControl*          prefix,
    const data::window& data);

  /// Constructor. Skips prefix.
  explicit ex_commandline_imp(
    ex_commandline*     cl,
    const std::string&  value = std::string(),
    const data::window& data  = data::window());

  /// other methods

  /// Handles string command.
  bool handle(const std::string& command);

  /// Handles char command.
  bool handle(char command);

  /// Handles keydown event.
  void on_key_down(wxKeyEvent& event);

  /// Virtual interface

  bool Destroy() override;

private:
  const wex::path& path() const final;

  void bind();
  bool handle_type(const std::string& command, const std::string& range);

  ex_commandline_input* cli();

  void ex_mode();
  void init();
  bool input_mode_finish() const;
  bool is_ex_mode() const;

  void on_char(wxKeyEvent& event);
  void on_key_down_control_r(wxKeyEvent& event);
  void on_key_down_escape();
  void on_key_down_page(wxKeyEvent& event);
  void on_key_down_tab();
  void on_text();
  void on_text_enter(wxEvent& event);
  void on_text_enter_do();
  bool on_text_enter_prep();

  void set_prefix();

  const int m_id_register;

  wxControl*      m_prefix{nullptr};
  ex_commandline* m_cl;

  char m_input{0};

  bool m_control_r{false}, m_mode_visual{false}, m_user_input{false};

  std::vector<ex_commandline_input*> m_clis;

  wex::path m_path;
};
}; // namespace wex
