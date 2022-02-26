////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp-on.cpp
// Purpose:   Implementation of wex::textctrl_imp class on.. methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/common/util.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/textctrl-input.h>
#include <wex/ui/textctrl.h>

#include "textctrl-imp.h"

void wex::textctrl_imp::on_char(wxKeyEvent& event)
{
  if (event.GetUnicodeKey() == WXK_NONE)
  {
    return;
  }

  switch (event.GetKeyCode())
  {
    case WXK_RETURN:
      event.Skip();

      if (m_input != 0)
      {
        m_command.handle(this, event.GetKeyCode());
      }
      break;

    case WXK_TAB:
      if (m_tc->stc() != nullptr && m_tc->stc()->path().file_exists())
      {
        path::current(m_tc->stc()->path().data().parent_path());
      }

      if (const auto& [r, e, v] = auto_complete_filename(m_command.command());
          r)
      {
        AppendText(e);
        m_command.append(e);
      }
      break;

    default:
      on_char_others(event);
  }
}

void wex::textctrl_imp::on_char_others(wxKeyEvent& event)
{
  bool skip = true;

  if (m_control_r)
  {
    skip             = false;
    const char     c = event.GetUnicodeKey();
    wxCommandEvent ce(wxEVT_MENU, m_id_register);

    if (c == '%')
    {
      if (m_tc->stc() != nullptr)
      {
        ce.SetString(m_tc->stc()->path().filename());
      }
    }
    else
    {
      ce.SetString(m_tc->stc()->vi_register(c));
    }

    if (!ce.GetString().empty())
    {
      m_command.insert(GetInsertionPoint(), std::string(1, WXK_CONTROL_R));
      m_command.insert(GetInsertionPoint() + 1, std::string(1, c));

      wxPostEvent(this, ce);
    }
  }

  m_user_input = true;
  m_control_r  = false;

  if (skip)
  {
    event.Skip();
    m_command.handle(this, event.GetKeyCode());
  }
}

void wex::textctrl_imp::on_key_down(wxKeyEvent& event)
{
  switch (event.GetKeyCode())
  {
    case 'r':
    case 'R':
      cut();

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
      break;

    case WXK_DOWN:
    case WXK_END:
    case WXK_HOME:
    case WXK_LEFT:
    case WXK_PAGEDOWN:
    case WXK_PAGEUP:
    case WXK_RIGHT:
    case WXK_UP:
      on_key_down_page(event);
      break;

    case WXK_BACK:
      m_command.handle(this, event.GetKeyCode());
      event.Skip();
      break;

    case WXK_ESCAPE:
      if (is_ex_mode())
      {
        Clear();
        m_command = ex_command(":");
      }
      else if (m_tc->stc() != nullptr)
      {
        m_tc->stc()->position_restore();
      }

      if (!is_ex_mode())
      {
        m_tc->get_frame()->show_ex_bar(frame::HIDE_BAR_FORCE_FOCUS_STC);
      }

      m_control_r  = false;
      m_user_input = false;
      break;

    default:
      if (isascii(event.GetKeyCode()) && event.GetKeyCode() != WXK_RETURN)
      {
        cut();
      }
      event.Skip();
  }
}

void wex::textctrl_imp::on_key_down_page(wxKeyEvent& event)
{
  switch (event.GetKeyCode())
  {
    case WXK_LEFT:
      if (m_all_selected)
      {
        SetInsertionPoint(0);
      }
      else if (const auto ip(GetInsertionPoint()); ip > 0)
      {
        SetInsertionPoint(ip - 1);
      }
      else
      {
        SelectNone();
      }

      m_all_selected = false;
      break;

    case WXK_RIGHT:
      if (m_all_selected)
      {
        SetInsertionPointEnd();
      }
      else if (const auto ip(GetInsertionPoint()); ip < GetLastPosition())
      {
        SetInsertionPoint(ip + 1);
      }
      else
      {
        SelectNone();
      }

      m_all_selected = false;
      break;

    default:
      if (m_command.type() == ex_command::type_t::FIND)
      {
        find_replace_data::get()->m_find_strings.set(event.GetKeyCode(), m_tc);
      }
      else if (m_input == 0)
      {
        if (const auto& val = tci()->get(); !val.empty() && GetValue().empty())
        {
          set_text(val);
          SelectAll();
        }
        else
        {
          tci()->set(event.GetKeyCode(), m_tc);
        }
      }
      else
      {
        event.Skip();
      }
  }
}

void wex::textctrl_imp::on_text(wxCommandEvent& event)
{
  event.Skip();

  if (get_text().size() == 0 && m_input == 0)
  {
    m_command.reset();
  }

  if (
    m_user_input && m_tc->stc() != nullptr &&
    m_command.type() == ex_command::type_t::FIND)
  {
    m_tc->stc()->position_restore();
    m_tc->stc()->find(
      get_text(),
      m_tc->stc()->vi_search_flags(),
      m_command.str() == "/");
  }
}

void wex::textctrl_imp::on_text_enter(wxCommandEvent& event)
{
  if (!on_text_enter_prep())
  {
    return;
  }

  if (input_mode_finish())
  {
    if (const auto text(m_command.command().substr(1, m_command.size() - 3));
        m_command.command() != ":." && !text.empty())
    {
      m_tc->stc()->vi_command(
        ":" + std::string(1, m_input) + "|" + text + m_tc->stc()->eol());
    }

    m_tc->get_frame()->show_ex_bar();
  }
  else if (m_input != 0)
  {
    event.Skip();
  }
  else if (
    (m_user_input && m_command.type() == ex_command::type_t::FIND) ||
    m_command.exec())
  {
    on_text_enter_do();
  }
}

void wex::textctrl_imp::on_text_enter_do()
{
  auto focus =
    (m_command.type() == ex_command::type_t::FIND ?
       frame::HIDE_BAR_FORCE_FOCUS_STC :
       frame::HIDE_BAR_FOCUS_STC);

  if (m_command.type() == ex_command::type_t::FIND)
  {
    find_replace_data::get()->set_find_string(get_text());
  }
  else
  {
    tci()->set(m_tc);

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
    Clear();
    m_command = ex_command(":");
    SetFocus();
  }

  if (m_input == 0 && !is_ex_mode())
  {
    m_tc->get_frame()->show_ex_bar(focus);
  }
}

bool wex::textctrl_imp::on_text_enter_prep()
{
  if (get_text().empty())
  {
    if (m_tc->stc() == nullptr)
    {
      log::debug("no stc");
      return false;
    }

    if (is_ex_mode())
    {
      m_command.reset();
      m_tc->stc()->vi_command(":.+1");
      SetFocus();
    }
    else
    {
      m_tc->get_frame()->show_ex_bar(frame::HIDE_BAR_FORCE_FOCUS_STC);
    }

    return false;
  }

  if (
    m_user_input && m_command.type() == ex_command::type_t::FIND &&
    m_tc->stc() != nullptr)
  {
    m_tc->stc()->vi_record(m_command.str() + get_text());
  }

  return true;
}

void wex::textctrl_imp::on_text_paste(wxCommandEvent& event)
{
  if (const auto text(clipboard_get()); !text.empty())
  {
    if (!GetStringSelection().empty())
    {
      m_command.handle(this, wxID_CUT);
    }

    m_command.insert(GetInsertionPoint(), text);
    event.Skip();
  }
}
