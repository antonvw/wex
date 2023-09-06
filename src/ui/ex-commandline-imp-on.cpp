////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline-imp-on.cpp
// Purpose:   Implementation of wex::ex_commandline_imp class on.. methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/factory/stc.h>
#include <wex/ui/ex-commandline-input.h>
#include <wex/ui/ex-commandline.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

#include "ex-commandline-imp.h"

void wex::ex_commandline_imp::ex_mode()
{
  ClearAll();

  m_command.set(":");

  SetFocus();
}

void wex::ex_commandline_imp::on_char(wxKeyEvent& event)
{
  if (event.GetUnicodeKey() == WXK_NONE)
  {
    return;
  }

  bool skip = true;

  if (m_control_r)
  {
    skip             = false;
    const char     c = event.GetUnicodeKey();
    wxCommandEvent ce(wxEVT_MENU, m_id_register);

    if (c == '%')
    {
      if (m_cl->stc() != nullptr)
      {
        ce.SetString(m_cl->stc()->path().filename());
      }
    }
    else
    {
      ce.SetString(m_cl->stc()->vi_register(c));
    }

    if (!ce.GetString().empty())
    {
      wxPostEvent(this, ce);
    }
  }

  m_user_input = true;
  m_control_r  = false;

  if (skip)
  {
    event.Skip();
  }
}

void wex::ex_commandline_imp::on_key_down(wxKeyEvent& event)
{
  switch (event.GetKeyCode())
  {
    case WXK_TAB:
      on_key_down_tab();
      break;

    case 'r':
    case 'R':
      on_key_down_control_r(event);
      break;

    case WXK_DOWN:
    case WXK_END:
    case WXK_HOME:
    case WXK_PAGEDOWN:
    case WXK_PAGEUP:
    case WXK_UP:
      on_key_down_page(event);
      break;

    case WXK_ESCAPE:
      on_key_down_escape();
      break;

    case WXK_RETURN:
      on_text_enter(event);
      break;

    default:
      if (isascii(event.GetKeyCode()))
      {
        Cut();
      }
      event.Skip();
  }
}

void wex::ex_commandline_imp::on_key_down_control_r(wxKeyEvent& event)
{
  Cut();

#ifdef __WXMAC__
  /* NOLINTNEXTLINE */
  if (event.GetModifiers() & wxMOD_RAW_CONTROL)
#else
  /* NOLINTNEXTLINE */
  if (event.GetModifiers() & wxMOD_CONTROL)
#endif
  {
    m_user_input = true;
    m_control_r  = true;
  }
  else
  {
    event.Skip();
  }
}

void wex::ex_commandline_imp::on_key_down_escape()
{
  if (is_ex_mode())
  {
    ex_mode();
  }
  else if (m_cl->stc() != nullptr)
  {
    m_cl->stc()->position_restore();
  }

  if (!is_ex_mode())
  {
    m_cl->get_frame()->show_ex_bar(frame::HIDE_BAR_FORCE_FOCUS_STC);
  }

  m_control_r  = false;
  m_user_input = false;
}

void wex::ex_commandline_imp::on_key_down_page(wxKeyEvent& event)
{
  if (m_command.type() == ex_command::type_t::FIND)
  {
    find_replace_data::get()->m_find_strings.set(
      event.GetKeyCode(),
      m_cl->control());
  }
  else if (m_input == 0)
  {
    if (m_clis.empty())
    {
      find_replace_data::get()->m_find_strings.set(
        event.GetKeyCode(),
        m_cl->control());
    }
    else if (const auto& val = cli()->get(); !val.empty() && get_text().empty())
    {
      set_text(val);
      SelectAll();
    }
    else
    {
      cli()->set(event.GetKeyCode(), m_cl->control());
    }
  }
  else
  {
    event.Skip();
  }
}

void wex::ex_commandline_imp::on_key_down_tab()
{
  if (m_cl->stc() != nullptr && m_cl->stc()->path().file_exists())
  {
    path::current(m_cl->stc()->path().data().parent_path());
  }

  if (const auto& [r, e, v] = auto_complete_filename(get_text()); r)
  {
    append_text(e);
    DocumentEnd();
  }
}

void wex::ex_commandline_imp::on_text()
{
  if (
    m_user_input && m_cl->stc() != nullptr &&
    m_command.type() == ex_command::type_t::FIND)
  {
    m_cl->stc()->position_restore();
    m_cl->stc()->find(
      get_text(),
      m_cl->stc()->vi_search_flags(),
      m_command.str() == "/");
  }
}

void wex::ex_commandline_imp::on_text_enter(wxEvent& event)
{
  if (!on_text_enter_prep())
  {
    return;
  }

  if (input_mode_finish())
  {
    if (const auto& text(get_text().substr(0, get_text().size() - 2));
        text != ":." && !text.empty())
    {
      m_cl->stc()->vi_command(line_data().command(
        ":" + std::string(1, m_input) + "|" + text + m_cl->stc()->eol()));
    }

    m_cl->get_frame()->show_ex_bar();

    if (is_ex_mode())
    {
      ClearAll();
      handle(":");
      m_cl->get_frame()->show_ex_bar(wex::frame::SHOW_BAR);
    }
  }
  else if (m_input != 0)
  {
    event.Skip();
  }
  else if (
    (m_command.type() == ex_command::type_t::FIND) ||
    m_command.reset(get_text()).exec())
  {
    on_text_enter_do();
  }
}

void wex::ex_commandline_imp::on_text_enter_do()
{
  auto focus =
    (m_command.type() == ex_command::type_t::FIND ?
       frame::HIDE_BAR_FORCE_FOCUS_STC :
       frame::HIDE_BAR_FOCUS_STC);

  if (m_command.type() == ex_command::type_t::FIND)
  {
    find_replace_data::get()->set_find_string(get_text());
    m_command.exec_finish(m_user_input);
  }
  else
  {
    cli()->set(m_cl);

    if (
      m_command.type() == ex_command::type_t::COMMAND ||
      m_command.type() == ex_command::type_t::COMMAND_EX)
    {
      if (
        get_text() == "gt" || get_text() == "n" || get_text() == "prev" ||
        get_text().starts_with("ta"))
      {
        focus = frame::HIDE_BAR_FORCE;
      }
      else if (get_text().starts_with("!"))
      {
        focus = frame::HIDE_BAR;
      }
    }
  }

  if (is_ex_mode())
  {
    ex_mode();
  }

  if (m_input == 0 && !is_ex_mode())
  {
    m_cl->get_frame()->show_ex_bar(focus);
  }
}

bool wex::ex_commandline_imp::on_text_enter_prep()
{
  if (get_text().empty())
  {
    if (m_cl->stc() == nullptr)
    {
      log::debug("no stc");
      return false;
    }

    if (is_ex_mode())
    {
      m_command.reset();
      m_cl->stc()->vi_command(line_data().command(":.+1"));
      SetFocus();
    }
    else
    {
      m_cl->get_frame()->show_ex_bar(frame::HIDE_BAR_FORCE_FOCUS_STC);
    }

    return false;
  }

  if (
    m_user_input && m_command.type() == ex_command::type_t::FIND &&
    m_cl->stc() != nullptr)
  {
    m_cl->stc()->vi_record(m_command.str() + get_text());
  }

  return true;
}
