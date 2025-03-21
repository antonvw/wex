////////////////////////////////////////////////////////////////////////////////
// Name:      shell.cpp
// Purpose:   Implementation of class wex::shell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/common/util.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/factory/bind.h>
#include <wex/factory/defs.h>
#include <wex/factory/process.h>
#include <wex/stc/auto-complete.h>
#include <wex/stc/shell.h>
#include <wex/ui/frame.h>

#include <numeric>
#include <ranges>

wex::shell::shell(
  const data::stc&   data,
  const std::string& prompt,
  const std::string& command_end)
  : stc(
      std::string(),
      data::stc(data).flags(
        data::stc::window_t().set(data::stc::WIN_NO_INDICATOR),
        data::control::OR))
  , m_command_end(command_end == std::string() ? eol() : command_end)
  , m_echo(true)
  , m_commands(config("Shell").get(config::strings_t{}))
  // Take care that m_commands_iterator is valid.
  , m_commands_iterator(m_commands.end())
  , m_commands_save_in_config(100)
  , m_prompt(prompt)
{
  // Override defaults from config.
  SetEdgeMode(wxSTC_EDGE_NONE);
  reset_margins(margin_t().set(MARGIN_FOLDING).set(MARGIN_LINENUMBER));

  AutoCompSetSeparator(3);

  // Start with a prompt.
  shell::prompt(std::string(), false);

  enable(true);

  auto_complete()->use(false); // we have our own auto_complete

  bind(this).command(
    {{[=, this](wxCommandEvent& event)
      {
        AppendText(event.GetString());
        get_frame()->output(event.GetString());
      },
      ID_SHELL_APPEND},
     {[=, this](wxCommandEvent& event)
      {
        AppendText(event.GetString());
      },
      ID_SHELL_APPEND_ERROR},
     {[=, this](wxCommandEvent& event)
      {
        AppendText(event.GetString());
      },
      ID_SHELL_COMMAND}});

  bind_other();
}

wex::shell::~shell() = default;

void wex::shell::AppendText(const wxString& text)
{
  const bool pos_at_end = (GetCurrentPos() >= GetTextLength());

  stc::AppendText(text);

  m_command_start_pos = GetTextLength();

  EmptyUndoBuffer();

  if (pos_at_end)
  {
    DocumentEnd();
    EnsureCaretVisible();
  }
}

void wex::shell::bind_other()
{
  Bind(
    wxEVT_CHAR,
    [=, this](wxKeyEvent& event)
    {
      if (m_enabled)
      {
        process_char(event.GetKeyCode());
      }
      if (m_echo)
      {
        event.Skip();
      }
    });

  Bind(
    wxEVT_KEY_DOWN,
    [=, this](wxKeyEvent& event)
    {
      on_key_down(event);
    });

  Bind(
    wxEVT_MIDDLE_UP,
    [=, this](wxMouseEvent& event)
    {
      on_mouse(event);
    });

  Bind(
    wxEVT_STC_CHARADDED,
    [=, this](wxStyledTextEvent& event)
    {
      if (!m_enabled)
      {
        event.Skip();
      }
      // do nothing, keep event from sent to stc.
    });

  Bind(
    wxEVT_STC_DO_DROP,
    [=, this](wxStyledTextEvent& event)
    {
      if (!m_enabled)
      {
        event.Skip();
      }
      event.SetDragResult(wxDragNone);
      event.Skip();
    });

  Bind(
    wxEVT_STC_START_DRAG,
    [=, this](wxStyledTextEvent& event)
    {
      if (!m_enabled)
      {
        event.Skip();
      }
      // Currently, no drag/drop, though we might be able to
      // drag/drop copy to command line.
      event.SetDragAllowMove(false);
      event.Skip();
    });
}

void wex::shell::enable(bool enabled)
{
  m_enabled = enabled;

  if (!m_enabled)
  {
    // A disabled shell follows vi mode.
    get_vi().use(
      config(_("stc.vi mode")).get(true) ? ex::mode_t::VISUAL :
                                           ex::mode_t::OFF);
  }
  else
  {
    // An enabled shell does not use vi mode.
    get_vi().use(ex::mode_t::OFF);
  }
}

void wex::shell::expand()
{
  // We might have commands:
  // 1) ls -l s
  // -> should build list with e.g. sample and src.
  // path:   s
  // prefix: s
  // subdir: empty
  // 2) ls -l src/
  // path:   src/
  // subdir: src
  // prefix: empty
  // -> should build list with all files in src
  // 3) ls -l src/vi
  // -> should build list with files in src starting with vi
  // path:   src/vi
  // subdir: src
  // prefix: vi
  wex::path   path(rfind_after(m_command, " "));
  std::string expansion;

  if (const auto prefix(path.filename()); AutoCompActive())
  {
    if (const auto index = AutoCompGetCurrent();
        index >= 0 && index < static_cast<int>(m_auto_complete_list.size()))
    {
      expansion = m_auto_complete_list[index].substr(prefix.length());
    }

    AutoCompCancel();
  }
  else if (const auto t = auto_complete_filename(m_command); t)
  {
    if (t->vector.size() > 1)
    {
      m_auto_complete_list = t->vector;
      AutoCompShow(
        prefix.length(),
        std::accumulate(
          t->vector.begin(),
          t->vector.end(),
          std::string(),
          [&](const std::string& a, const std::string& b)
          {
            return a.empty() ?
                     b :
                     a + static_cast<char>(AutoCompGetSeparator()) + b;
          }));
    }
    else
    {
      expansion = t->expansion;
    }
  }

  if (!expansion.empty())
  {
    m_command += expansion;

    // We cannot use our AppendText, as command start pos
    // should not be changed.
    wex::stc::AppendText(expansion);
    DocumentEnd();
  }
}

const std::string wex::shell::get_command() const
{
  return !m_commands.empty() ? m_commands.back() : std::string();
}

const std::string wex::shell::get_history() const
{
  return std::accumulate(m_commands.begin(), m_commands.end(), std::string());
}

void wex::shell::keep_command()
{
  // Prevent large commands, in case command end is not eol.
  if (m_command_end != eol())
  {
    boost::algorithm::trim(m_command);
  }

  m_commands.emplace_back(m_command);

  if (m_commands_save_in_config > 0)
  {
    config("Shell").set(m_commands);
  }
}

void wex::shell::on_mouse(wxMouseEvent& event)
{
  if (event.MiddleUp())
  {
    if (CanCopy())
    {
      Copy();
      Paste();
    }
  }
  else
  {
    event.Skip();
  }
}

void wex::shell::Paste()
{
  if (!CanPaste())
  {
    return;
  }

  // Take care that we cannot paste somewhere inside.
  if (GetCurrentPos() < m_command_start_pos)
  {
    DocumentEnd();
  }

  wex::stc::Paste();

  m_command += clipboard_get();
}

bool wex::shell::process_char(int key)
{
  bool processed = false;

  // No need to check m_enabled, already done by calling this method.
  switch (key)
  {
    case WXK_RETURN:
      if (AutoCompActive())
      {
        if (!m_auto_complete_list.empty())
        {
          expand();
        }
        else
        {
          processed = true;
        }
      }
      else if (m_command.empty())
      {
        send_command();
      }
      else if (m_command_end == eol() || m_command.ends_with(m_command_end))
      {
        // We have a command.
        EmptyUndoBuffer();

        // History command.
        if (
          m_command ==
          "history" + (m_command_end == eol() ? std::string() : m_command_end))
        {
          keep_command();
          show_history();
          prompt();
        }
        // !.. command, get it from history.
        else if (m_command.substr(0, 1) == "!")
        {
          if (set_command_from_history(m_command.substr(1)))
          {
            AppendText(eol() + m_command);
            keep_command();
            send_command();
          }
        }
        // Other command.
        else
        {
          keep_command();
          send_command();
        }

        m_command.clear();
      }
      else
      {
        process_char_default(key);
        processed = true;
      }

      m_commands_iterator = m_commands.end();
      break;

    case WXK_BACK:
    case WXK_DELETE:
      // Delete the key at current position.
      if (const int offset = (key == WXK_BACK ? 1 : 0),
          index            = GetCurrentPos() - m_command_start_pos - offset;
          !m_command.empty() && index >= 0 &&
          index < static_cast<int>(m_command.length()))
      {
        m_command.erase(index, 1);
      }
      break;

    case WXK_TAB:
      expand();
      break;

    default:
      process_char_default(key);
      processed = true;
  }

  return processed;
}

void wex::shell::process_char_default(int key)
{
  // Insert the key at current position.
  if (const int index = GetCurrentPos() - m_command_start_pos;
      GetCurrentPos() < GetLength() && index >= 0 &&
      index < static_cast<int>(m_command.size()))
  {
    m_command.insert(index, 1, static_cast<char>(key));
  }
  else
  {
    m_command.append(1, static_cast<char>(key));
  }
}

bool wex::shell::prompt(const std::string& text, bool add_eol)
{
  if (!m_enabled)
  {
    return false;
  }

  bool appended = false;

  if (!text.empty())
  {
    appended = true;
    AppendText(text);
  }

  if (add_eol)
  {
    appended = true;
    AppendText(eol());
  }

  if (!m_prompt.empty())
  {
    appended = true;
    AppendText(m_prompt);
  }

  DocumentEnd();

  if (!appended)
  {
    EmptyUndoBuffer();
  }

  m_command_start_pos = GetCurrentPos();

  return true;
}

void wex::shell::send_command()
{
  if (m_process != nullptr && m_process->is_running())
  {
    AppendText(eol());
    m_process->write(m_command.empty() ? "\n" : m_command);
  }
  else
  {
    log::trace("posted") << m_command;
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
    event.SetString(m_command);
    GetParent()->GetEventHandler()->AddPendingEvent(event);
    GetParent()->GetEventHandler()->ProcessPendingEvents();
  }
}

bool wex::shell::set_command_from_history(const std::string& short_command)
{
  try
  {
    const auto no_asked_for = std::stoi(short_command);

    for (int no = 1; const auto& it : m_commands)
    {
      if (no == no_asked_for)
      {
        m_command = it;
        return true;
      }

      no++;
    }
  }
  catch (std::exception&)
  {
    std::string short_command_check;

    if (m_command_end == eol())
    {
      short_command_check = short_command;
    }
    else
    {
      short_command_check = short_command.substr(
        0,
        short_command.length() - m_command_end.length());
    }

    if (std::ranges::any_of(
          std::views::reverse(m_commands),
          [&short_command_check, this](const auto& it)
          {
            if (it.substr(0, short_command_check.size()) == short_command_check)
            {
              m_command = it;
              return true;
            }
            return false;
          }))
    {
      return true;
    }
  }

  prompt(eol() + short_command + ": event not found");

  return false;
}

void wex::shell::set_process(factory::process* process)
{
  clear();

  m_process = process;

  if (process != nullptr)
  {
    enable(true);
    process->set_handler_out(this);
    SetName(process->data().exe());

    get_lexer().set(
      process->data().exe().starts_with("gcc") ? "errorlist" : "bash");
  }
}

bool wex::shell::set_prompt(const std::string& prompt, bool do_prompt)
{
  if (!m_enabled)
  {
    return false;
  }

  m_prompt = prompt;

  if (do_prompt)
  {
    shell::prompt(
      std::string(),
      // only add eol if text is present
      GetTextLength() > 0);
  }

  return true;
}

void wex::shell::show_command(int key)
{
  SetTargetRange(m_command_start_pos, GetTextLength());

  if (key == WXK_UP)
  {
    if (m_commands_iterator != m_commands.begin())
    {
      --m_commands_iterator;
    }
  }
  else
  {
    if (m_commands_iterator != m_commands.end())
    {
      ++m_commands_iterator;
    }
  }

  if (m_commands_iterator != m_commands.end())
  {
    m_command = *m_commands_iterator;
    ReplaceTarget(m_command);
  }
  else
  {
    Undo();
  }

  DocumentEnd();
}

void wex::shell::show_history()
{
  for (int command_no = 1; const auto& it : m_commands)
  {
    AppendText("\n" + std::to_string(command_no++) + " " + it);
  }
}

void wex::shell::Undo()
{
  if (CanUndo())
  {
    wex::stc::Undo();
  }

  m_command.clear();
}
