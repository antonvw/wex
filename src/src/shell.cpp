////////////////////////////////////////////////////////////////////////////////
// Name:      shell.cpp
// Purpose:   Implementation of class wex::shell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <functional>
#include <numeric>
#include <wex/shell.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/process.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

wex::shell::shell(
  const stc_data& data,
  const std::string& prompt, 
  const std::string& command_end,
  bool echo, 
  const std::string& lexer,
  int commands_save_in_config)
  : stc(std::string(), stc_data(data).
      flags(stc_data::window_t().set(stc_data::WIN_NO_INDICATOR), control_data::OR))
  , m_command_end(command_end == std::string() ? eol(): command_end)
  , m_echo(echo)
  , m_commands(config("Shell").get(std::list< std::string> {}))
    // Take care that m_commands_iterator is valid.
  , m_commands_iterator(m_commands.end())
  , m_commands_save_in_config(commands_save_in_config)
  , m_prompt(prompt)
{
  // Override defaults from config.
  SetEdgeMode(wxSTC_EDGE_NONE);
  reset_margins(margin_t().set(MARGIN_FOLDING).set(MARGIN_LINENUMBER));

  AutoCompSetSeparator(3);

  // Start with a prompt.
  shell::prompt(std::string(), false);

  enable(true);
  
  auto_complete().use(false); // we have our own autocomplete

  get_lexer().set(lexer);
  
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (m_enabled)
    {
      process_char(event.GetKeyCode());
    }
    if (m_echo)
    {
      event.Skip();
    }});

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    AppendText(event.GetString());}, ID_SHELL_APPEND);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    AppendText(event.GetString());}, ID_SHELL_APPEND_ERROR);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    AppendText(event.GetString());}, ID_SHELL_COMMAND);
  
  Bind(wxEVT_MIDDLE_UP, [=](wxMouseEvent& event) {
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
    }});
    
  Bind(wxEVT_KEY_DOWN, [=](wxKeyEvent& event) {
    if (!m_enabled)
    {
      if (get_vi().mode().insert())
      {
        DocumentEnd();
        get_vi().mode().escape();
      }
      if (GetCurrentPos() >= m_command_start_pos && 
          (m_process == nullptr || m_process->is_running()))
      {
        enable(true);
      }
      else
      {
        event.Skip();
        return;
      }
    }
    else
    {
      if (config(_("vi mode")).get(true) && (GetCurrentPos() < m_command_start_pos))
      {
        enable(false);
        event.Skip();
        return;
      }
    }

    bool skip = true;
    
    switch (const int key = event.GetKeyCode(); key)
    {
      case WXK_RETURN:
      case WXK_TAB:
        if (m_echo && process_char(key)) event.Skip();
        break;
      
      // Up or down key pressed, and at the end of document (and autocomplete active)
      case WXK_UP:
      case WXK_DOWN:
        if (GetCurrentPos() == GetTextLength() && !AutoCompActive())
        {
          show_command(key);
        }
        else
        {
          event.Skip();
        }
        break;
        
      case WXK_HOME:
        Home();
        if (GetLine(GetCurrentLine()).StartsWith(m_prompt))
        {
          GotoPos(GetCurrentPos() + m_prompt.length());
        }
        break;
        
      // Shift-Insert key pressed, used for pasting.
      case WXK_INSERT:
        if (event.GetModifiers() == wxMOD_SHIFT)
        {
          Paste();
        }
        break;
        
      // Middle mouse button, to paste, though actually OnMouse is used.
      case WXK_MBUTTON:
        Paste();
        break;
        
      // Backspace or delete key pressed.
      case WXK_BACK:
      case WXK_DELETE:
        if (GetCurrentPos() <= m_command_start_pos)
        {
          // Ignore, so do nothing.
        }
        else
        {
          // Allow.
          process_char(key);
          if (m_echo) event.Skip();
        }
        break;
        
      case WXK_ESCAPE:
        if (AutoCompActive())
        {
          AutoCompCancel();
        }
        else
        {
          event.Skip();
        }
        break;
        
      default:
        // Ctrl-C pressed.
        if (event.GetModifiers() == wxMOD_CONTROL && key == 'C')
        {
          skip = false;
          if (m_process != nullptr)
          {
            m_process->stop();
          }
        }
        // Ctrl-Q pressed, used to stop processing.
        else if (event.GetModifiers() == wxMOD_CONTROL && key == 'Q')
        {
          skip = false;
          if (m_process != nullptr)
          {
            m_process->stop();
          }
          else
          {
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND_STOP);
            wxPostEvent(GetParent(), event);
          }
        }
        // Ctrl-V pressed, used for pasting.
        else if (event.GetModifiers() == wxMOD_CONTROL && key == 'V')
        {
          Paste();
        }
        // If we enter regular text and not already building a command, first goto end.
        else if (event.GetModifiers() == wxMOD_NONE &&
          key < WXK_START &&
          GetCurrentPos() < m_command_start_pos)
        {
          DocumentEnd();
        }
        m_commands_iterator = m_commands.end();
        if (m_echo && skip) event.Skip();
    }});

  Bind(wxEVT_STC_CHARADDED, [=](wxStyledTextEvent& event) {
    if (!m_enabled)
    {
      event.Skip();
    }
    // do nothing, keep event from sent to stc.
    });
  
  Bind(wxEVT_STC_DO_DROP, [=](wxStyledTextEvent& event) {
    if (!m_enabled)
    {
      event.Skip();
    }
    event.SetDragResult(wxDragNone);
    event.Skip();});
  
  Bind(wxEVT_STC_START_DRAG,  [=](wxStyledTextEvent& event) {
    if (!m_enabled)
    {
      event.Skip();
    }
    // Currently no drag/drop, though we might be able to
    // drag/drop copy to command line.
    event.SetDragAllowMove(false);
    event.Skip();});
}

wex::shell::~shell()
{
  if (m_commands_save_in_config > 0)
  {
    config("Shell").set(m_commands);
  }
}

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

void wex::shell::enable(bool enabled)
{
  m_enabled = enabled;
  
  if (!m_enabled)
  {
    // A disabled shell follows vi mode.
    get_vi().use(config(_("vi mode")).get(true));
  }
  else
  {
    // An enabled shell does not use vi mode.
    get_vi().use(false);
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
  wex::path path(after(m_command, ' ', false));
  std::string expansion;
  
  if (const auto prefix(path.fullname()); AutoCompActive())
  {
    if (const auto index = AutoCompGetCurrent(); 
      index >= 0 && index < (int)m_auto_complete_list.size())
    {
      expansion = m_auto_complete_list[index].substr(prefix.length());
    }
    
    AutoCompCancel();
  }
  else if (const auto [r, e, v] = autocomplete_filename(m_command); r)
  {
    if (v.size() > 1)
    {
      m_auto_complete_list = v;
      AutoCompShow(prefix.length(), std::accumulate(
        v.begin(), v.end(), std::string(), 
        [&](const std::string& a, const std::string& b) {
          return a.empty() ? b : a + (char)AutoCompGetSeparator() + b;}));
    }
    else 
    {
      expansion = e;
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
  return !m_commands.empty() ? m_commands.back(): std::string();
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
    m_command = trim(m_command);
  }
  
  m_commands.emplace_back(m_command);
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
      else if (
        m_command_end == eol() ||
        wxString(m_command).EndsWith(m_command_end))
      {
        // We have a command.
        EmptyUndoBuffer();
        
        // History command.
        if (m_command == "history" +
           (m_command_end == eol() ? std::string(): m_command_end))
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
      if (const int offset = (key == WXK_BACK ? 1: 0), 
        index = GetCurrentPos() - m_command_start_pos - offset;
        !m_command.empty() && index >= 0 && index < (int)m_command.length())
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
    GetCurrentPos() < GetLength() && 
    index >= 0 && index < (int)m_command.size())
  {
    m_command.insert(index, 1, char(key));
  }
  else
  {
    m_command.append(1, char(key));
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
    m_process->write(m_command.empty() ? "\n": m_command);
  }
  else
  {
    log::verbose("posted") << m_command;
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
    int no = 1;

    for (const auto& it : m_commands)
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
      short_command_check =
        short_command.substr(
          0,
          short_command.length() - m_command_end.length());
    }

    for (
      auto it = m_commands.rbegin();
      it != m_commands.rend();
      ++it)
    {
      if (const auto command = *it;
        command.substr(0, short_command_check.size()) == short_command_check)
      {
        m_command = command;
        return true;
      }
    }
  }
  

  prompt(eol() + short_command + ": event not found");

  return false;
}

void wex::shell::set_process(process* process)
{
  m_process = process;

  wex::path::current(get_filename().get_path());
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
    shell::prompt();
  }
  
  return true;
}

void wex::shell::show_command(int key)
{
  SetTargetStart(m_command_start_pos);
  SetTargetEnd(GetTextLength());

  if (key == WXK_UP)
  {
    if (m_commands_iterator != m_commands.begin())
    {
      m_commands_iterator--;
    }
  }
  else
  {
    if (m_commands_iterator != m_commands.end())
    {
      m_commands_iterator++;
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
  int command_no = 1;

  for (const auto& it : m_commands)
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
