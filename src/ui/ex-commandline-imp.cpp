////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline-imp.cpp
// Purpose:   Implementation of wex::ex_commandline_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <utility>

#include <wex/core/config.h>
#include <wex/factory/bind.h>
#include <wex/ui/ex-commandline-input.h>
#include <wex/ui/ex-commandline.h>
#include <wex/ui/frame.h>
#include <wx/settings.h>

#include "ex-commandline-imp.h"

wex::ex_commandline_imp::ex_commandline_imp(
  ex_commandline*     cl,
  wxControl*          prefix,
  const data::window& data)
  : syntax::stc(data)
  , m_id_register(NewControlId())
  , m_prefix(prefix)
  , m_cl(cl)
  , m_clis{
      new ex_commandline_input(ex_command::type_t::COMMAND, "ex-cmd.command"),
      new ex_commandline_input(ex_command::type_t::CALC, "ex-cmd.calc"),
      new ex_commandline_input(
        ex_command::type_t::COMMAND_EX,
        "ex-cmd.command-ex"),
      new ex_commandline_input(
        ex_command::type_t::COMMAND_RANGE,
        "ex-cmd.command-ex-range"),
      new ex_commandline_input(ex_command::type_t::ESCAPE, "ex-cmd.escape"),
      new ex_commandline_input(
        ex_command::type_t::ESCAPE_RANGE,
        "ex-cmd.escape-range"),
      new ex_commandline_input(
        ex_command::type_t::FIND_MARGIN,
        "ex-cmd.margin")}
{
  init();
  bind();
}

wex::ex_commandline_imp::ex_commandline_imp(
  ex_commandline*     cl,
  const std::string&  value,
  const data::window& data)
  : syntax::stc(data)
  , m_id_register(0)
  , m_cl(cl)
{
  init();

  set_text(value);

  m_command.no_type();
  m_command.set(value);
}

void wex::ex_commandline_imp::bind()
{
  bind_wx();

  wex::bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        WriteText(event.GetString());
      },
      m_id_register}});

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
    wxEVT_SET_FOCUS,
    [=, this](wxFocusEvent& event)
    {
      event.Skip();

      m_cl->get_frame()->set_find_focus(this);

      if (m_cl->stc() != nullptr)
      {
        m_cl->stc()->position_save();
      }
    });

  Bind(
    wxEVT_STC_CHARADDED,
    [=, this](wxStyledTextEvent& event)
    {
      event.Skip();
      on_text();
    });
}

wex::ex_commandline_input* wex::ex_commandline_imp::cli()
{
  return std::to_underlying(m_command.type()) >=
             std::to_underlying(ex_command::type_t::NONE) ?
           m_clis[std::to_underlying(ex_command::type_t::COMMAND)] :
           m_clis[std::to_underlying(m_command.type())];
}

bool wex::ex_commandline_imp::Destroy()
{
  for (auto it : m_clis)
  {
    delete it;
  }

  return factory::stc::Destroy();
}

bool wex::ex_commandline_imp::handle(const std::string& command)
{
  const auto& range(!command.empty() ? command.substr(1) : std::string());

  m_user_input = false;

  if (m_cl->stc() != nullptr)
  {
    m_command = ex_command(m_cl->stc()->get_ex_command()).set(command);
    get_lexer().set(m_cl->stc()->get_lexer().display_lexer());
  }
  else
  {
    m_command.reset(command);
    get_lexer().set(std::string());
  }

  m_input       = 0;
  m_mode_visual = !range.empty();
  m_control_r   = false;

  set_prefix();

  m_cl->get_frame()->pane_set(
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

bool wex::ex_commandline_imp::handle(char command)
{
  m_control_r = false;
  m_input     = command;

  get_lexer().set(
    m_cl->stc() != nullptr ? m_cl->stc()->get_lexer().display_lexer() :
                             std::string());

  ClearAll();
  m_command.reset();

  m_cl->get_frame()->pane_set(
    "VIBAR",
    wxAuiPaneInfo().BestSize(
      -1,
      4 * GetFont().GetPixelSize().GetHeight() + 10));

  return true;
}

bool wex::ex_commandline_imp::handle_type(
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
      set_text(cli()->get());
      SelectAll();
      break;

    case ex_command::type_t::COMMAND:
    case ex_command::type_t::COMMAND_EX:
      if (command == ":!")
      {
        set_text("!");
        SetInsertionPointEnd();
      }
      else if (const auto& current(cli()->get()); !current.empty())
      {
        const auto is_address(
          m_cl->get_frame()->is_address(m_cl->stc(), current));

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
      if (m_cl->stc() != nullptr)
      {
        set_text(
          !m_mode_visual || range == ex_command::selection_range() ?
            m_cl->stc()->get_find_string() :
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

void wex::ex_commandline_imp::init()
{
  SetUseHorizontalScrollBar(false);
  SetUseVerticalScrollBar(false);
  SetFont(config(_("stc.Text font"))
            .get(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

  get_lexer().set(lexer(this));
  reset_margins();
}

bool wex::ex_commandline_imp::input_mode_finish() const
{
  if (const auto& text(get_text()); m_input == 0 || text.size() < 2)
  {
    return false;
  }
  else
  {
    const auto& last_two(text.substr(text.size() - 2, 2));
    return text == ":." || last_two == "\n." || last_two == "\r.";
  }
}

bool wex::ex_commandline_imp::is_ex_mode() const
{
  return m_cl->stc() != nullptr && !m_cl->stc()->is_visual();
}

const wex::path& wex::ex_commandline_imp::path() const
{
  return m_path;
}

void wex::ex_commandline_imp::set_prefix()
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
