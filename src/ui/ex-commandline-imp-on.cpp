////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline-imp-on.cpp
// Purpose:   Implementation of wex::ex_commandline_imp class on.. methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2024 Anton van Wezenbeek
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

// see also commands-motion.cpp
#define MOTION(DIRECTION)                                                      \
  if (event.ControlDown() || event.RawControlDown())                           \
  {                                                                            \
    if (event.ShiftDown())                                                     \
      Word##DIRECTION##Extend();                                               \
    else                                                                       \
      Word##DIRECTION();                                                       \
  }                                                                            \
  else                                                                         \
  {                                                                            \
    event.Skip();                                                              \
  }

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
    skip = false;

    if (wxCommandEvent ce(wxEVT_MENU, m_id_register); m_cl->stc() != nullptr)
    {
      const char c = event.GetUnicodeKey();
      ce.SetString(
        (c == '%') ? m_cl->stc()->path().filename() :
                     m_cl->stc()->vi_register(c));

      wxPostEvent(this, ce);
    }

    m_text_not_expanded += std::string(1, WXK_CONTROL_R);
  }

  m_user_input = true;
  m_control_r  = false;

  if (skip)
  {
    event.Skip();
  }

  m_text_not_expanded += std::string(1, event.GetUnicodeKey());
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

    case WXK_END:
    case WXK_HOME:
      if (!event.ShiftDown() && event.HasAnyModifiers())
      {
        on_key_down_page(event);
      }
      else
      {
        event.Skip();
      }
      break;

    case WXK_LEFT:
      // see also vi convert_key_event
      MOTION(Left);
      break;

    case WXK_RIGHT:
      MOTION(Right);
      break;

    case WXK_DOWN:
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
      event.Skip();
  }
}

void wex::ex_commandline_imp::on_key_down_control_r(wxKeyEvent& event)
{
  Cut();

#ifdef __WXOSX__
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
  else if (m_text_input == 0)
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

  if (const auto& r(auto_complete_filename(get_text())); r)
  {
    append_text(r->expansion);
    DocumentEnd();
  }
}

void wex::ex_commandline_imp::on_text()
{
  if (m_user_input && m_command.type() == ex_command::type_t::FIND)
  {
    m_cl->find();
  }
}

void wex::ex_commandline_imp::on_text_enter(wxEvent& event)
{
  if (m_cl->stc() == nullptr || !on_text_enter_prep())
  {
    return;
  }

  if (text_input_mode_finish())
  {
    if (const auto& text(
          m_cl->stc()->vi_is_recording() ?
            m_text_not_expanded :
            get_text().substr(0, get_text().size() - 2));
        text != "." && !text.empty())
    {
      m_cl->stc()->vi_command(
        line_data().command(":" + std::string(1, m_text_input) + "|" + text));
      m_text_not_expanded.clear();
    }

    m_cl->get_frame()->show_ex_bar();

    if (is_ex_mode())
    {
      ClearAll();
      handle(":");
      m_cl->get_frame()->show_ex_bar(wex::frame::SHOW_BAR);
    }
  }
  else if (m_text_input != 0)
  {
    event.Skip();
  }
  else if (
    (m_command.type() == ex_command::type_t::FIND) ||
    m_command
      .reset(m_cl->stc()->vi_is_recording() ? m_text_not_expanded : get_text())
      .exec())
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

    if (m_cl->find_on_enter())
    {
      return;
    }

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

  if (m_text_input == 0 && !is_ex_mode())
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
