////////////////////////////////////////////////////////////////////////////////
// Name:      vi.cpp
// Purpose:   Implementation of class wex::vi
//            http://pubs.opengroup.org/onlinepubs/9699919799/utilities/vi.html
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/vi/vi.h>

#include "vim.h"

// without this code adding tab in block insert mode fails, it only
// add one tab instead of a line of tabs
namespace wex
{
bool is_block_insert(vi* vi)
{
  return vi->mode().is_insert() &&
         (vi->get_stc()->SelectionIsRectangle() ||
          vi->get_stc()->GetSelectionMode() == wxSTC_SEL_THIN);
}

bool is_special_key(const wxKeyEvent& event, const vi_mode& mode)
{
  return !event.HasAnyModifiers() &&
         (event.GetKeyCode() == WXK_ESCAPE || event.GetKeyCode() == WXK_BACK ||
          event.GetKeyCode() == WXK_RETURN ||
          event.GetKeyCode() == WXK_NUMPAD_ENTER ||
          event.GetKeyCode() == WXK_TAB ||
          (!mode.is_insert() &&
           (event.GetKeyCode() == WXK_LEFT ||
            event.GetKeyCode() == WXK_DELETE ||
            event.GetKeyCode() == WXK_DOWN || event.GetKeyCode() == WXK_UP ||
            event.GetKeyCode() == WXK_RIGHT ||
            event.GetKeyCode() == WXK_PAGEUP ||
            event.GetKeyCode() == WXK_PAGEDOWN)));
}

bool process_modifier(vi* vi, macros::key_t type, const wxKeyEvent& event)
{
  if (const auto& it =
        ex::get_macros().get_keys_map(type).find(event.GetKeyCode());
      it != ex::get_macros().get_keys_map(type).end())
  {
    vi->command(it->second);
    return false;
  }

  return true;
}

bool visual_ex_command(const std::string& command, ex* ex)
{
  const auto& sel_command(
    command[0] + ex_command::selection_range() + command.substr(1));

  return ex->command(sel_command);
}

bool visual_vi_command(const std::string& command, vi* vi)
{
  if (
    vi->mode().is_visual() &&
    command.find(ex_command::selection_range()) == std::string::npos &&
    !vi->get_stc()->get_selected_text().empty() && command[0] == ':')
  {
    return visual_ex_command(command, vi);
  }

  return false;
}
} // namespace wex

wex::vi::vi(wex::factory::stc* arg, mode_t ex_mode)
  : ex(arg, ex_mode)
  , m_mode(
      this,
      // insert mode process
      [=, this](const std::string& command)
      {
        if (!m_dot)
        {
          m_insert_text.clear();
        }

        get_stc()->BeginUndoAction();
      },
      // back to command mode process
      [=, this]()
      {
        if (!m_dot)
        {
          const std::string c(m_insert_command + register_insert());
          set_last_command(c + esc());
          get_macros().record(c);
          get_macros().record(esc(), true);
        }
        m_command.clear();
        m_insert_command.clear();
        get_stc()->EndUndoAction();
      })
  , m_last_commands{{"!", "<", ">", "A", "C", "D", "I", "J", "O",
                     "P", "R", "S", "X", "Y", "a", "c", "d", "g",
                     "i", "o", "p", "r", "s", "x", "y", "~"}}
  , m_motion_commands(commands_motion())
  , m_other_commands(commands_other())
{
}

void wex::vi::append_insert_command(const std::string& s)
{
  m_insert_command.append(s);
  log::trace("insert command") << m_insert_command;
}

void wex::vi::append_insert_text(const std::string& s)
{
  m_insert_text.append(s);
  set_register_insert(m_insert_text);
  log::trace("insert text") << m_insert_text;
}

bool wex::vi::command(const std::string& command)
{
  if (command.empty() || !is_active())
  {
    return false;
  }

  m_command_string = command;

  if (command.front() != ':' && command.front() != '!')
  {
    log::trace("vi command") << command;
  }

  if (visual_vi_command(command, this))
  {
    return auto_write();
  }
  else if (m_mode.is_insert())
  {
    return insert_mode(command);
  }
  else if (
    !m_dot && command.size() > 2 && command.back() == WXK_ESCAPE &&
    command[command.size() - 2] == WXK_ESCAPE)
  {
    m_mode.escape();
    m_command.clear();
    return auto_write();
  }

  if (auto parse(command); !parse_command(parse))
  {
    return false;
  }
  else
  {
    if (!m_dot)
    {
      set_last_command(command);
    }

    if (
      !m_mode.is_insert() && command[0] != 'q' && command[0] != ':' &&
      command[0] != '!' && command != "/" && command != "?" &&
      command != k_s(WXK_CONTROL_R) + "=")
    {
      get_macros().record(command);
    }

    return auto_write();
  }
}

void wex::vi::command_reg(const std::string& reg)
{
  switch (reg[0])
  {
    case 0:
      break;

    // calc register: control-R =
    case WXK_CONTROL_R:
      if (reg.size() > 1 && reg[1] == '=' && get_stc()->is_visual())
      {
        if (reg.size() == 2)
        {
          set_register_insert(std::string());
          frame()->show_ex_command(get_stc(), reg);
        }
        else
        {
          const auto sum = calculator(reg.substr(2));

          if (m_mode.is_insert())
          {
            if (m_last_command.find('c') != std::string::npos)
            {
              get_stc()->ReplaceSelection(wxEmptyString);
            }

            get_stc()->add_text(std::to_string(sum));

            append_insert_command(reg);
          }
          else
          {
            set_register_yank(std::to_string(sum));
            frame()->show_ex_message(std::to_string(sum));
          }
        }
      }
      else
      {
        frame()->show_ex_message("calc register is control-R =");
      }
      break;

    // clipboard register
    case '*':
      if (m_mode.is_insert())
      {
        put(true);
      }
      break;

    // filename register
    case '%':
      if (m_mode.is_insert())
      {
        get_stc()->add_text(get_stc()->path().filename());
      }
      else
      {
        frame()->show_ex_message(get_stc()->path().string());
        clipboard_add(get_stc()->path().string());
      }
      break;

    default:
      if (!get_macros().get_register(reg[0]).empty())
      {
        if (m_mode.is_insert())
        {
          get_stc()->add_text(get_macros().get_register(reg[0]));

          if (reg[0] == '.')
          {
            append_insert_text(register_insert());
          }
        }
        else
        {
          frame()->show_ex_message(get_macros().get_register(reg[0]));
        }
      }
      else
      {
        frame()->show_ex_message("?" + reg);
      }
  }
}

char wex::vi::convert_key_event(const wxKeyEvent& event) const
{
  if (event.GetKeyCode() == WXK_BACK)
    return WXK_BACK;
  else if (event.GetKeyCode() == WXK_RETURN && !m_mode.is_insert())
    return 'j';
  else if (event.GetModifiers() & wxMOD_RAW_CONTROL)
    return event.GetKeyCode();

  char c = event.GetUnicodeKey();

  if (c == WXK_NONE)
  {
    switch (event.GetKeyCode())
    {
      case WXK_LEFT:
        c = 'h';
        break;
      case WXK_DOWN:
        c = 'j';
        break;
      case WXK_UP:
        c = 'k';
        break;
      case WXK_RIGHT:
        c = 'l';
        break;
      case WXK_DELETE:
        c = 'x';
        break;
      case WXK_PAGEUP:
        c = WXK_CONTROL_B;
        break;
      case WXK_PAGEDOWN:
        c = WXK_CONTROL_F;
        break;
      case WXK_NUMPAD_ENTER:
        c = 'j';
        break;
      default:
        c = event.GetKeyCode();
    }
  }

  return c;
}

bool wex::vi::delete_range(int start, int end)
{
  if (get_stc()->GetReadOnly())
  {
    return false;
  }

  const auto first = (start < end ? start : end);
  const auto last  = (start < end ? end : start);

  if (get_stc()->is_hexmode())
  {
    get_stc()->get_hexmode_erase(last - first, first);
  }
  else if (
    get_stc()->GetSelectionMode() == wxSTC_SEL_RECTANGLE &&
    !get_stc()->GetSelectedText().empty())
  {
    get_stc()->Cut();
  }
  else
  {
    const auto& b(get_stc()->GetTextRangeRaw(first, last));

    get_macros().set_register(
      register_name() ? register_name() : '0',
      std::string(b.data(), b.length()));

    get_stc()->DeleteRange(first, last - first);
  }

  return true;
}

void wex::vi::filter_count(std::string& command)
{
  /*
   command: 3w
   -> v has 2 elements
   -> m_count 3
   -> command w
   */
  if (regex v("^([1-9][0-9]*)(.*)"); v.match(command) == 2)
  {
    try
    {
      m_count_present  = true;
      const auto count = std::stoi(v[0]);
      m_count *= count;
      append_insert_command(v[0]);
      command = v[1];
    }
    catch (std::exception& e)
    {
      m_count_present = false;
      log(e) << command;
      command = v[1];
    }
  }
}

wex::vi::motion_t wex::vi::get_motion(const std::string& command) const
{
  switch (command[0])
  {
    case 'c':
      return motion_t::CHANGE;

    case 'd':
      return motion_t::DEL;

    case 'g':
      return vim::get_motion(command);

    case 'y':
      return motion_t::YANK;

    default:
      return motion_t::NAVIGATE;
  }
}

bool wex::vi::insert_mode(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  else if (get_stc()->is_hexmode())
  {
    return insert_mode_hex(command);
  }
  // add control chars
  else if (command.size() == 2 && command[1] == 0)
  {
    append_insert_text(std::string(1, command[0]));
    get_stc()->add_text(std::string(1, command[0]));
    return true;
  }

  if (command.starts_with(k_s(WXK_CONTROL_R) + "="))
  {
    command_reg(command);
    return true;
  }
  else if (command.find(k_s(WXK_CONTROL_R)) != std::string::npos)
  {
    return insert_mode_register(command);
  }

  insert_mode_other(command);

  return true;
}

void wex::vi::insert_mode_escape(const std::string& command)
{
  // Add extra inserts if necessary.
  if (!m_insert_text.empty())
  {
    for (auto i = 1; i < m_count; i++)
    {
      insert_mode_normal(m_insert_text);
    }

    set_register_insert(m_insert_text);
  }

  // If we have text to be added.
  if (command.size() > 1)
  {
    if (const auto rest(command.substr(0, command.size() - 1));
        !get_stc()->GetSelectedText().empty())
    {
      get_stc()->ReplaceSelection(rest);
    }
    else
    {
      if (!get_stc()->GetOvertype())
      {
        REPEAT(get_stc()->add_text(rest));
      }
      else
      {
        std::string text;
        get_stc()->SetTargetStart(get_stc()->GetCurrentPos());
        REPEAT(text += rest;);
        get_stc()->SetTargetEnd(get_stc()->GetCurrentPos() + text.size());
        get_stc()->ReplaceTarget(text);
      }
    }
  }

  if (m_mode.escape())
  {
    get_stc()->SetOvertype(false);
  }
}

bool wex::vi::insert_mode_hex(const std::string& command)
{
  if (static_cast<int>(command.back()) == WXK_ESCAPE)
  {
    if (m_mode.escape())
    {
      get_stc()->SetOvertype(false);
    }

    return true;
  }
  else
  {
    return get_stc()->get_hexmode_insert(command, get_stc()->GetCurrentPos());
  }
}

void wex::vi::insert_mode_normal(const std::string& text)
{
  if (boost::tokenizer<boost::char_separator<char>> tok(
        text,
        boost::char_separator<char>("", "\r\n", boost::keep_empty_tokens));
      text.find('\0') == std::string::npos &&
      std::distance(tok.begin(), tok.end()) >= 1)
  {
    for (auto it = tok.begin(); it != tok.end(); ++it)
    {
      if (auto token(*it); !token.empty())
      {
        if (token.back() == ' ' || token.back() == '\t' || token.back() == ';')
        {
          if (!m_insert_text.empty())
          {
            const auto last(m_insert_text.find_last_of(" ;\t"));

            if (const auto& it = get_macros().get_abbreviations().find(
                  m_insert_text.substr(last + 1));
                it != get_macros().get_abbreviations().end())
            {
              m_insert_text.replace(last + 1, it->first.size(), it->second);

              const auto pos       = get_stc()->GetCurrentPos();
              const auto match_pos = get_stc()->FindText(
                pos,
                get_stc()->PositionFromLine(get_stc()->get_current_line()),
                it->first);

              if (match_pos != wxSTC_INVALID_POSITION)
              {
                get_stc()->SetTargetRange(
                  match_pos,
                  match_pos + it->first.size());
                get_stc()->ReplaceTarget(it->second);
                get_stc()->SetCurrentPos(
                  pos + it->second.size() - it->first.size());
              }
            }
          }
          else
          {
            const auto last(token.find_last_of(" ;\t", token.size() - 2));
            const auto word(token.substr(last + 1, token.size() - 2 - last));

            if (const auto& it = get_macros().get_abbreviations().find(word);
                it != get_macros().get_abbreviations().end())
            {
              token.replace(last + 1, it->first.size(), it->second);
            }
          }
        }

        get_stc()->add_text(token);

        if (token == "\n")
        {
          get_stc()->auto_indentation(token[0]);
        }
      }
    }
  }
  else
  {
    get_stc()->add_text(text);
  }
}

bool wex::vi::insert_mode_other(const std::string& command)
{
  switch (command.back())
  {
    case WXK_BACK:
      if (!m_insert_text.empty())
      {
        m_insert_text.pop_back();
      }
      get_stc()->DeleteBack();
      break;

    case WXK_CONTROL_R:
      append_insert_text(command);
      break;

    case WXK_DELETE:
      delete_range(get_stc()->GetCurrentPos(), get_stc()->GetCurrentPos() + 1);
      break;

    case WXK_ESCAPE:
      insert_mode_escape(command);
      break;

    default:
      if (
        m_last_command.find('c') != std::string::npos && m_insert_text.empty())
      {
        get_stc()->ReplaceSelection(wxEmptyString);
      }

      if (!m_insert_text.empty() && m_insert_text.back() == WXK_CONTROL_R)
      {
        get_stc()->ReplaceSelection(wxEmptyString);

        if (command.back() != '.')
        {
          append_insert_text(command);
        }
        else
        {
          m_insert_text.pop_back();
        }

        command_reg(std::string(1, command.back()));
        return false;
      }
      else
      {
        if (
          command.size() == 1 &&
          (static_cast<int>(command.back()) == WXK_RETURN ||
           static_cast<int>(command.back()) == WXK_NUMPAD_ENTER))
        {
          get_stc()->NewLine();

          if (!get_stc()->AutoCompActive())
          {
            append_insert_text(get_stc()->eol());
          }
        }
        else
        {
          if (!get_stc()->GetOvertype())
          {
            insert_mode_normal(command);
          }

          if (!m_dot)
          {
            append_insert_text(command);
          }
        }
      }
  }

  return true;
}

bool wex::vi::insert_mode_register(const std::string& command)
{
  if (command.size() < 2)
  {
    return false;
  }

  std::string text(command);
  marker_and_register_expansion(this, text);
  insert_mode(text);

  return true;
}

bool wex::vi::on_char(const wxKeyEvent& event)
{
  if (!is_active())
  {
    return true;
  }
  else if (m_mode.is_insert())
  {
    if (is_block_insert(this))
    {
      return true;
    }

    m_command.append(convert_key_event(event));
    const bool result = insert_mode(m_command.command());

    if (result || (get_stc()->is_hexmode() && m_command.size() > 2))
    {
      m_command.clear();
    }

    return result && get_stc()->GetOvertype();
  }
  else
  {
    if (!(event.GetModifiers() & wxMOD_ALT))
    {
      // This check is important, as WXK_NONE (0)
      // would add nullptr terminator at the end of m_command,
      // and pressing ESC would not help, (rest is empty
      // because of the nullptr).
      if (event.GetUnicodeKey() != (wxChar)WXK_NONE)
      {
        if (
          !m_command.empty() && m_command.front() == '@' &&
          event.GetKeyCode() == WXK_BACK)
        {
          m_command.pop_back();
        }
        else
        {
#ifdef __WXOSX__
          if (event.GetModifiers() & wxMOD_RAW_CONTROL)
          {
            if (m_command.append_exec(event.GetKeyCode()))
            {
              m_command.clear();
            }
          }
          else
#endif
            if (m_command.append_exec(event.GetUnicodeKey()))
          {
            m_command.clear();
          }
        }
      }
      else
      {
        return true;
      }

      return false;
    }
    else
    {
      return true;
    }
  }
}

bool wex::vi::on_key_down(const wxKeyEvent& event)
{
  if (!is_active() || get_stc()->AutoCompActive())
  {
    return true;
  }
  else if (!m_command.empty() && m_command.front() == '@')
  {
    return process_macro_key(event);
  }
  else if (is_block_insert(this) && event.GetKeyCode() != WXK_ESCAPE)
  {
    m_command.clear();
    return true;
  }
  else if (is_special_key(event, m_mode))
  {
    return process_special_key(event);
  }
  else if (
    (event.GetModifiers() & wxMOD_CONTROL) && event.GetKeyCode() != WXK_NONE)
  {
    return process_modifier(this, macros::key_t::KEY_CONTROL, event);
  }
  else if ((event.GetModifiers() & wxMOD_ALT) && event.GetKeyCode() != WXK_NONE)
  {
    if (!m_mode.is_command())
    {
      command(esc());
    }

    return process_modifier(this, macros::key_t::KEY_ALT, event);
  }
  else
  {
    return true;
  }
}

bool wex::vi::process_macro_key(const wxKeyEvent& event)
{
  if (event.GetKeyCode() == WXK_BACK)
  {
    m_command.pop_back();
    frame()->statustext(m_command.command().substr(1), "PaneMacro");
  }
  else if (event.GetKeyCode() == WXK_ESCAPE)
  {
    m_command.clear();
    m_insert_command.clear();
    frame()->statustext(get_macros().mode().get_macro(), "PaneMacro");
  }

  return true;
}

bool wex::vi::process_special_key(const wxKeyEvent& event)
{
  if (event.GetKeyCode() == WXK_BACK)
  {
    if (!m_insert_text.empty())
    {
      m_insert_text.pop_back();
    }
  }
  else if (m_command.append_exec(convert_key_event(event)))
  {
    m_command.clear();

    if (!m_mode.is_insert())
    {
      m_insert_command.clear();
    }

    return false;
  }

  return true;
}

bool wex::vi::put(bool after)
{
  if (register_text().empty())
  {
    return false;
  }

  // do not trim
  const bool yanked_lines = (get_number_of_lines(register_text(), false) > 1);

  if (yanked_lines)
  {
    if (after)
    {
      if (
        get_stc()->GetColumn(get_stc()->GetCurrentPos()) > 0 ||
        get_stc()->GetSelectedText().empty())
      {
        get_stc()->LineDown();
      }
    }

    get_stc()->Home();
  }

  get_stc()->add_text(register_text());

  if (yanked_lines && after)
  {
    get_stc()->LineUp();
  }

  return true;
}

void wex::vi::set_last_command(const std::string& command)
{
  size_t first = 0;

  if (regex v("^([1-9][0-9]*)(.*)"); v.match(command) == 2)
  {
    first = v[0].size(); // skip a possible leading count
  }

  if (const auto& it = std::find(
        m_last_commands.begin(),
        m_last_commands.end(),
        command.substr(first, 1));
      it != m_last_commands.end())
  {
    if (command != "gg")
    {
      m_last_command = command;
      log::trace("last command") << m_last_command;
    }
  }
}

void wex::vi::yank_range(int start)
{
  if (auto end = get_stc()->GetCurrentPos(); end - start > 0)
  {
    get_stc()->CopyRange(start, end - start);
    get_stc()->SetSelection(start, end);
  }
  else
  {
    // reposition end at start of selection
    if (!get_stc()->GetSelectedText().empty())
    {
      end = get_stc()->GetSelectionStart();
    }
    else
    {
      end--;
    }

    get_stc()->CopyRange(end, start - end);
    get_stc()->SetSelection(end, start);
  }

  m_mode.escape();

  if (!register_name())
  {
    set_register_yank(get_stc()->get_selected_text());
  }
  else
  {
    get_macros().set_register(register_name(), get_stc()->get_selected_text());
    get_stc()->SelectNone();
  }

  info_message(get_stc()->get_selected_text(), wex::info_message_t::YANK);
}
