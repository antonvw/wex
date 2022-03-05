////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp.cpp
// Purpose:   Implementation of wex::textctrl_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/textctrl-input.h>
#include <wex/ui/textctrl.h>
#include <wx/control.h>
#include <wx/settings.h>

#include "textctrl-imp.h"

wex::textctrl_imp::textctrl_imp(
  textctrl*           tc,
  wxControl*          prefix,
  const data::window& data)
  : wxTextCtrl(
      data.parent(),
      data.id(),
      wxEmptyString,
      data.pos(),
      data.size(),
      // msw shows scrollbar, wxTE_NO_VSCROLL hides that, but then :i
      // no longer is ok
      data.style() | wxTE_PROCESS_ENTER | wxTE_MULTILINE)
  , m_id_register(NewControlId())
  , m_prefix(prefix)
  , m_tc(tc)
  , m_timer(this)
  , m_tcis{
      new textctrl_input(ex_command::type_t::COMMAND, "ex-cmd.command"),
      new textctrl_input(ex_command::type_t::CALC, "ex-cmd.calc"),
      new textctrl_input(ex_command::type_t::COMMAND_EX, "ex-cmd.command-ex"),
      new textctrl_input(
        ex_command::type_t::COMMAND_RANGE,
        "ex-cmd.command-ex-range"),
      new textctrl_input(ex_command::type_t::ESCAPE, "ex-cmd.escape"),
      new textctrl_input(
        ex_command::type_t::ESCAPE_RANGE,
        "ex-cmd.escape-range"),
      new textctrl_input(ex_command::type_t::FIND_MARGIN, "ex-cmd.margin")}
{
  SetFont(config(_("stc.Text font"))
            .get(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

  bind();
  bind_timer();
}

wex::textctrl_imp::textctrl_imp(
  textctrl*           tc,
  const std::string&  value,
  const data::window& data)
  : wxTextCtrl(
      data.parent(),
      data.id(),
      value,
      data.pos(),
      data.size(),
      data.style() | wxTE_PROCESS_ENTER)
  , m_id_register(0)
  , m_tc(tc)
  , m_timer(this)
{
  m_command.no_type();
  m_command.set(value);

  bind_timer();
}

void wex::textctrl_imp::bind()
{
  Bind(
    wxEVT_CHAR,
    [=, this](wxKeyEvent& event)
    {
      on_char(event);
    });

  Bind(
    wxEVT_KEY_DOWN,
    [=, this](wxKeyEvent& event)
    {
      on_key_down(event);
    });

  Bind(
    wxEVT_MENU,
    [=, this](wxCommandEvent& event)
    {
      WriteText(event.GetString());
    },
    m_id_register);

  Bind(
    wxEVT_SET_FOCUS,
    [=, this](wxFocusEvent& event)
    {
      event.Skip();

      if (m_tc->stc() != nullptr)
      {
        m_tc->stc()->position_save();
      }
    });

  Bind(
    wxEVT_TEXT,
    [=, this](wxCommandEvent& event)
    {
      on_text(event);
    });

  Bind(
    wxEVT_TEXT_CUT,
    [=, this](wxClipboardTextEvent& event)
    {
      // prevent cut
    });

  Bind(
    wxEVT_TEXT_ENTER,
    [=, this](wxCommandEvent& event)
    {
      on_text_enter(event);
    });

  Bind(
    wxEVT_TEXT_PASTE,
    [=, this](wxClipboardTextEvent& event)
    {
      on_text_paste(event);
    });
}

void wex::textctrl_imp::bind_timer()
{
  Bind(
    wxEVT_TIMER,
    [=, this](wxTimerEvent& event)
    {
      wxTextCtrl::SelectAll();
      m_all_selected = true;
    });
}

void wex::textctrl_imp::cut()
{
  if (!GetStringSelection().empty())
  {
    m_command.handle(this, wxID_CUT);

    Cut();
  }
}

bool wex::textctrl_imp::Destroy()
{
  for (auto it : m_tcis)
  {
    delete it;
  }

  return wxTextCtrl::Destroy();
}

// A GetValue().ToStdString() should suffice, but that
// corrupts a " character.
const std::string wex::textctrl_imp::get_text()
{
  if (is_ex_mode())
  {
    if (!m_command.empty() && m_command.front() != ':')
    {
      m_command = ex_command(":");
    }
  }

  return m_prefix == nullptr ?
           GetValue().ToStdString() :
           m_command.command().substr(m_command.str().size());
}

bool wex::textctrl_imp::handle(const std::string& command)
{
  const auto range(!command.empty() ? command.substr(1) : std::string());

  m_user_input = false;

  if (m_tc->stc() != nullptr)
  {
    m_command = ex_command(m_tc->stc()->get_ex_command()).set(command);
  }
  else
  {
    m_command.reset(command, true);
  }

  m_input       = 0;
  m_mode_visual = !range.empty();
  m_control_r   = false;

  set_prefix();

  m_tc->get_frame()->pane_set(
    "VIBAR",
    wxAuiPaneInfo().BestSize(-1, GetFont().GetPixelSize().GetHeight() + 10));

  if (!handle_type(command, range))
  {
    return false;
  }

  Show();
  SetFocus();

  return true;
}

bool wex::textctrl_imp::handle(char command)
{
  m_control_r = false;
  m_input     = command;

  Clear();

  m_command.reset();

  m_tc->get_frame()->pane_set(
    "VIBAR",
    wxAuiPaneInfo().BestSize(
      -1,
      4 * GetFont().GetPixelSize().GetHeight() + 10));

  return true;
}

bool wex::textctrl_imp::handle_type(
  const std::string& command,
  const std::string& range)
{
  switch (m_command.type())
  {
    case ex_command::type_t::CALC:
    case ex_command::type_t::COMMAND_RANGE:
    case ex_command::type_t::ESCAPE:
    case ex_command::type_t::ESCAPE_RANGE:
    case ex_command::type_t::FIND_MARGIN:
      set_text(tci()->get());
      SelectAll();
      break;

    case ex_command::type_t::COMMAND:
    case ex_command::type_t::COMMAND_EX:
      if (command == ":!")
      {
        set_text("!");
        SetInsertionPointEnd();
      }
      else if (const auto& current(tci()->get()); !current.empty())
      {
        const auto is_address(
          m_tc->get_frame()->is_address(m_tc->stc(), current));

        set_text(
          m_mode_visual && current.find(range) != 0 && is_address ?
            range + current :
            current);
        SelectAll();
      }
      else
      {
        set_text(range);
        SelectAll();
      }
      break;

    case ex_command::type_t::FIND:
      if (m_tc->stc() != nullptr)
      {
        set_text(
          !m_mode_visual || range == ex_command::selection_range() ?
            m_tc->stc()->get_find_string() :
            std::string());
      }
      else
      {
        set_text(std::string());
      }

      SelectAll();
      break;

    default:
      return false;
  }

  return true;
}

bool wex::textctrl_imp::input_mode_finish() const
{
  if (m_input == 0 || m_command.size() < 2)
  {
    return false;
  }

  const auto last_two(m_command.command().substr(m_command.size() - 2, 2));

  return m_command.command() == ":." || last_two == ".\n" || last_two == ".\r";
}

bool wex::textctrl_imp::is_ex_mode() const
{
  return m_tc->stc() != nullptr && !m_tc->stc()->is_visual();
}

void wex::textctrl_imp::SelectAll()
{
  if (
    m_command.command().find("\"") != std::string::npos ||
    m_command.command().find("\'") != std::string::npos)
  {
    m_timer.StartOnce(500);
  }
  else
  {
    wxTextCtrl::SelectAll();
    m_all_selected = true;
  }
}

void wex::textctrl_imp::set_prefix()
{
  if (m_prefix != nullptr && !m_command.str().empty())
  {
    m_prefix->SetLabelText(
      m_command.type() == ex_command::type_t::COMMAND_RANGE ||
          m_command.type() == ex_command::type_t::ESCAPE_RANGE ?
        m_command.str() :
        std::string(1, m_command.str().back()));
  }
}

void wex::textctrl_imp::set_text(const std::string& text)
{
  m_command.reset(text);
  ChangeValue(text);
}

wex::textctrl_input* wex::textctrl_imp::tci()
{
  return static_cast<int>(m_command.type()) >=
             static_cast<int>(ex_command::type_t::NONE) ?
           m_tcis[static_cast<int>(ex_command::type_t::COMMAND)] :
           m_tcis[static_cast<int>(m_command.type())];
}
