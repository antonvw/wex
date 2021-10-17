////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl-imp.cpp
// Purpose:   Implementation of wex::textctrl_imp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
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
  , m_calcs(new textctrl_input(ex_command::type_t::CALC))
  , m_commands(new textctrl_input(ex_command::type_t::COMMAND))
  , m_commands_ex(new textctrl_input(ex_command::type_t::COMMAND_EX))
  , m_escapes(new textctrl_input(ex_command::type_t::ESCAPE))
  , m_find_margins(new textctrl_input(ex_command::type_t::FIND_MARGIN))
{
  SetFont(config(_("stc.Text font"))
            .get(wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));

  Bind(
    wxEVT_CHAR,
    [=, this](wxKeyEvent& event)
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

          if (const auto& [r, e, v] =
                auto_complete_filename(m_command.command());
              r)
          {
            AppendText(e);
            m_command.append(e);
          }
          break;

        default:
          bool skip = true;

          if (m_control_r)
          {
            skip             = false;
            const char     c = event.GetUnicodeKey();
            wxCommandEvent event(wxEVT_MENU, m_id_register);

            if (c == '%')
            {
              if (m_tc->stc() != nullptr)
              {
                event.SetString(m_tc->stc()->path().filename());
              }
            }
            else
            {
              event.SetString(m_tc->stc()->vi_register(c));
            }

            if (!event.GetString().empty())
            {
              m_command.insert(
                GetInsertionPoint(),
                std::string(1, WXK_CONTROL_R));
              m_command.insert(GetInsertionPoint() + 1, std::string(1, c));

              wxPostEvent(this, event);
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
    });

  Bind(
    wxEVT_KEY_DOWN,
    [=, this](wxKeyEvent& event)
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
              else if (const auto ip(GetInsertionPoint());
                       ip < GetLastPosition())
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
                find_replace_data::get()->m_find_strings.set(
                  event.GetKeyCode(),
                  m_tc);
              }
              else if (m_input == 0)
              {
                if (const auto& val = tci()->get();
                    !val.empty() && GetValue().empty())
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
      if (get_text().empty())
      {
        if (m_tc->stc() == nullptr)
        {
          log::debug("no stc");
          return;
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
        return;
      }

      if (
        m_user_input && m_command.type() == ex_command::type_t::FIND &&
        m_tc->stc() != nullptr)
      {
        m_tc->stc()->vi_record(m_command.str() + get_text());
      }

      if (input_mode_finish())
      {
        if (const auto text(
              m_command.command().substr(1, m_command.size() - 3));
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
        int focus =
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
    });

  Bind(
    wxEVT_TEXT_PASTE,
    [=, this](wxClipboardTextEvent& event)
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
    });

  bind();
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

  bind();
}

bool wex::textctrl_imp::Destroy()
{
  delete m_calcs;
  delete m_commands;
  delete m_commands_ex;
  delete m_escapes;
  delete m_find_margins;

  return wxTextCtrl::Destroy();
}

void wex::textctrl_imp::bind()
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

    wxTextCtrl::Cut();
  }
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

  m_tc->get_frame()->pane_set(
    "VIBAR",
    wxAuiPaneInfo().BestSize(-1, GetFont().GetPixelSize().GetHeight() + 10));

  if (m_prefix != nullptr && !m_command.str().empty())
  {
    m_prefix->SetLabel(std::string(1, m_command.str().back()));
  }

  switch (m_command.type())
  {
    case ex_command::type_t::CALC:
    case ex_command::type_t::ESCAPE:
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
      else if (!tci()->get().empty())
      {
        set_text(
          m_mode_visual && tci()->get().find(range) != 0 ?
            range + tci()->get() :
            tci()->get());
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
          !m_mode_visual ? m_tc->stc()->get_find_string() : std::string());
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

void wex::textctrl_imp::set_text(const std::string& text)
{
  m_command.reset(text);
  ChangeValue(text);
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

wex::textctrl_input* wex::textctrl_imp::tci()
{
  switch (m_command.type())
  {
    case ex_command::type_t::CALC:
      return m_calcs;

    case ex_command::type_t::COMMAND_EX:
      return m_commands_ex;

    case ex_command::type_t::ESCAPE:
      return m_escapes;

    case ex_command::type_t::FIND_MARGIN:
      return m_find_margins;

    default:
      return m_commands;
  }
};
