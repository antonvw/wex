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
#include <wex/defs.h> // for ID_SHELL_COMMAND
#include <wex/process.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <easylogging++.h>

wex::shell::shell(
  const stc_data& data,
  const std::string& prompt, 
  const std::string& command_end,
  bool echo, 
  const std::string& lexer,
  int commands_save_in_config)
  : stc(std::string(), stc_data(data).
      flags(stc_data::window_t().set(stc_data::WIN_NO_INDICATOR), control_data::OR))
  , m_CommandEnd(command_end == std::string() ? eol(): command_end)
  , m_Echo(echo)
  , m_CommandsSaveInConfig(commands_save_in_config)
  , m_Prompt(prompt)
{
  // Override defaults from config.
  SetEdgeMode(wxSTC_EDGE_NONE);
  reset_margins(margin_t().set(MARGIN_FOLDING).set(MARGIN_LINENUMBER));

  AutoCompSetSeparator(3);

  // Start with a prompt.
  shell::prompt(std::string(), false);

  if (m_CommandsSaveInConfig > 0)
  {
    // Get all previous commands.
    for (tokenizer tkz(
      config("Shell").get(),
      std::string(1, AutoCompGetSeparator()));
      tkz.has_more_tokens(); )
    {
      m_Commands.push_front(tkz.get_next_token());
    }
  }

  // Take care that m_CommandsIterator is valid.
  m_CommandsIterator = m_Commands.end();
  
  enable(true);
  
  auto_complete().use(false); // we have our own autocomplete

  get_lexer().set(lexer);
  
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (m_Enabled)
    {
      process_char(event.GetKeyCode());
    }
    if (m_Echo)
    {
      event.Skip();
    }});

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
    if (!m_Enabled)
    {
      if (get_vi().mode().insert())
      {
        DocumentEnd();
        get_vi().mode().escape();
      }
      if (GetCurrentPos() >= m_CommandStartPosition && 
          (m_Process == nullptr || m_Process->is_running()))
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
      if (config(_("vi mode")).get(true) && (GetCurrentPos() < m_CommandStartPosition))
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
        if (m_Echo && process_char(key)) event.Skip();
        break;
      
      // Up or down key pressed, and at the end of document (and autocomplete active)
      case WXK_UP:
      case WXK_DOWN:
        if (GetCurrentPos() == GetTextLength() && !AutoCompActive())
        {
          ShowCommand(key);
        }
        else
        {
          event.Skip();
        }
        break;
        
      case WXK_HOME:
        Home();
        if (GetLine(GetCurrentLine()).StartsWith(m_Prompt))
        {
          GotoPos(GetCurrentPos() + m_Prompt.length());
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
        if (GetCurrentPos() <= m_CommandStartPosition)
        {
          // Ignore, so do nothing.
        }
        else
        {
          // Allow.
          process_char(key);
          if (m_Echo) event.Skip();
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
          if (m_Process != nullptr)
          {
            m_Process->kill(SIGINT);
          }
        }
        // Ctrl-Q pressed, used to stop processing.
        else if (event.GetModifiers() == wxMOD_CONTROL && key == 'Q')
        {
          skip = false;
          if (m_Process != nullptr)
          {
            m_Process->kill();
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
          GetCurrentPos() < m_CommandStartPosition)
        {
          DocumentEnd();
        }
        m_CommandsIterator = m_Commands.end();
        if (m_Echo && skip) event.Skip();
    }});

  Bind(wxEVT_STC_CHARADDED, [=](wxStyledTextEvent& event) {
    if (!m_Enabled)
    {
      event.Skip();
    }
    // do nothing, keep event from sent to stc.
    });
  
  Bind(wxEVT_STC_DO_DROP, [=](wxStyledTextEvent& event) {
    if (!m_Enabled)
    {
      event.Skip();
    }
#if wxUSE_DRAG_AND_DROP
    event.SetDragResult(wxDragNone);
#endif    
    event.Skip();});
  
  Bind(wxEVT_STC_START_DRAG,  [=](wxStyledTextEvent& event) {
    if (!m_Enabled)
    {
      event.Skip();
    }
    // Currently no drag/drop, though we might be able to
    // drag/drop copy to command line.
#if wxUSE_DRAG_AND_DROP
    event.SetDragAllowMove(false);
#endif    
    event.Skip();});
}

wex::shell::~shell()
{
  if (m_CommandsSaveInConfig > 0)
  {
    std::string values;
    int items = 0;

    for (
      auto it = m_Commands.rbegin(); 
      it != m_Commands.rend() && items < m_CommandsSaveInConfig;
      ++it)
    {
      values += *it + (char)AutoCompGetSeparator();
      items++;
    }

    config("Shell").set(values);
  }
}

void wex::shell::AppendText(const wxString& text)
{
  const bool pos_at_end = (GetCurrentPos() >= GetTextLength());

  stc::AppendText(text);
  
  m_CommandStartPosition = GetTextLength();
  
  EmptyUndoBuffer();
  
  if (pos_at_end)
  {
    DocumentEnd();
    EnsureCaretVisible();
  }
}

void wex::shell::enable(bool enabled)
{
  m_Enabled = enabled;
  
  if (!m_Enabled)
  {
    // A disabled shell follows STC vi mode.
    get_vi().use(config(_("vi mode")).get(true));
  }
  else
  {
    // An enabled shell does not use vi mode.
    get_vi().use(false);
  }
}

void wex::shell::Expand()
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
  wex::path path(after(m_Command, ' ', false));
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
  else if (const auto [r, e, v] = autocomplete_filename(m_Command); r)
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
    m_Command += expansion;
    
    // We cannot use our AppendText, as command start pos
    // should not be changed.
    wex::stc::AppendText(expansion);
    DocumentEnd();
  }
}
    
const std::string wex::shell::get_command() const
{
  return !m_Commands.empty() ? m_Commands.back(): std::string();
}

const std::string wex::shell::get_history() const
{
  return std::accumulate(m_Commands.begin(), m_Commands.end(), std::string());
}

void wex::shell::KeepCommand()
{
  // Prevent large commands, in case command end is not eol.
  if (m_CommandEnd != eol())
  {
    m_Command = skip_white_space(m_Command);
  }
  
  m_Commands.emplace_back(m_Command);
}

void wex::shell::Paste()
{
  if (!CanPaste())
  {
    return;
  }
  
  // Take care that we cannot paste somewhere inside.
  if (GetCurrentPos() < m_CommandStartPosition)
  {
    DocumentEnd();
  }
  
  wex::stc::Paste();
  
  m_Command += clipboard_get();  
}

bool wex::shell::process_char(int key)
{
  bool processed = false;
  
  // No need to check m_Enabled, already done by calling this method.
  switch (key)
  {
    case WXK_RETURN:
      if (AutoCompActive())
      {
        if (!m_auto_complete_list.empty())
        {
          Expand();
        }
        else
        {
          processed = true;
        }
      }
      else if (m_Command.empty())
      {
        if (m_Process != nullptr)
        {
          AppendText(eol());
          m_Process->write(m_Command);
        }
        else
        {
          wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
          event.SetString(m_Command);
          wxPostEvent(GetParent(), event);
        }
      }
      else if (
        m_CommandEnd == eol() ||
        wxString(m_Command).EndsWith(m_CommandEnd))
      {
        // We have a command.
        EmptyUndoBuffer();
        
        // History command.
        if (m_Command == "history" +
           (m_CommandEnd == eol() ? std::string(): m_CommandEnd))
        {
          KeepCommand();
          ShowHistory();
          prompt();
        }
        // !.. command, get it from history.
        else if (m_Command.substr(0, 1) == "!")
        {
          if (SetCommandFromHistory(m_Command.substr(1)))
          {
            AppendText(eol() + m_Command);
            KeepCommand();
          
            if (m_Process != nullptr)
            {
              AppendText(eol());
              m_Process->write(m_Command);
            }
            else
            {
              wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
              event.SetString(m_Command);
              wxPostEvent(GetParent(), event);
            }
          }
          else
          {
            prompt(eol() + m_Command + ": event not found");
          }
        }
        // Other command, send to parent or process.
        else
        {
          KeepCommand();
          
          if (m_Process != nullptr)
          {
            AppendText(eol());
            m_Process->write(m_Command);
          }
          else
          {
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
            event.SetString(m_Command);
            wxPostEvent(GetParent(), event);
          }
        }

        m_Command.clear();
      }
      else
      {
        process_char_default(key);
        processed = true;
      }

      m_CommandsIterator = m_Commands.end();
      break;
  
    case WXK_BACK:
    case WXK_DELETE:
      // Delete the key at current position.
      if (const int offset = (key == WXK_BACK ? 1: 0), 
        index = GetCurrentPos() - m_CommandStartPosition - offset;
        !m_Command.empty() && index >= 0 && index < (int)m_Command.length())
      {
        m_Command.erase(index, 1);
      }
      break;
    
    case WXK_TAB:
      Expand();
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
  if (const int index = GetCurrentPos() - m_CommandStartPosition;
    GetCurrentPos() < GetLength() && 
    index >= 0 && index < (int)m_Command.size())
  {
    m_Command.insert(index, 1, char(key));
  }
  else
  {
    m_Command += wxChar(key);
  }
}
  
bool wex::shell::prompt(const std::string& text, bool add_eol)
{
  if (!m_Enabled)
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

  if (!m_Prompt.empty())
  {
    appended = true;
    AppendText(m_Prompt);
  }
  
  DocumentEnd();
  
  if (!appended)
  {
    EmptyUndoBuffer();
  }
  
  m_CommandStartPosition = GetCurrentPos();
  
  return true;
}

bool wex::shell::SetCommandFromHistory(const std::string& short_command)
{
  if (const auto no_asked_for = atoi(short_command.c_str()); no_asked_for > 0)
  {
    int no = 1;

    for (const auto& it : m_Commands)
    {
      if (no == no_asked_for)
      {
        m_Command = it;
        return true;
      }

      no++;
    }
  }
  else
  {
    std::string short_command_check;

    if (m_CommandEnd == eol())
    {
      short_command_check = short_command;
    }
    else
    {
      short_command_check =
        short_command.substr(
          0,
          short_command.length() - m_CommandEnd.length());
    }

    for (
      auto it = m_Commands.rbegin();
      it != m_Commands.rend();
      ++it)
    {
      if (const auto command = *it;
        command.substr(0, short_command_check.size()) == short_command_check)
      {
        m_Command = command;
        return true;
      }
    }
  }

  return false;
}

void wex::shell::set_process(process* process)
{
  m_Process = process;
}

bool wex::shell::set_prompt(const std::string& prompt, bool do_prompt) 
{
  if (!m_Enabled)
  {
    return false;
  }
  
  m_Prompt = prompt;
  
  if (do_prompt) 
  {
    shell::prompt();
  }
  
  return true;
}

void wex::shell::ShowCommand(int key)
{
  SetTargetStart(m_CommandStartPosition);
  SetTargetEnd(GetTextLength());

  if (key == WXK_UP)
  {
    if (m_CommandsIterator != m_Commands.begin())
    {
      m_CommandsIterator--;
    }
  }
  else
  {
    if (m_CommandsIterator != m_Commands.end())
    {
      m_CommandsIterator++;
    }
  }

  if (m_CommandsIterator != m_Commands.end())
  {
    m_Command = *m_CommandsIterator;
    ReplaceTarget(m_Command);
  }
  else
  {
    Undo();
  }

  DocumentEnd();
}

void wex::shell::ShowHistory()
{
  int command_no = 1;

  for (const auto& it : m_Commands)
  {
    AppendText(wxString::Format("\n%d %s", command_no++, it.c_str()));
  }
}

void wex::shell::Undo()
{
  if (CanUndo())
  {
    wex::stc::Undo();
  }
  
  m_Command.clear();
}
