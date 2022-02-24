////////////////////////////////////////////////////////////////////////////////
// Name:      shell-keys.cpp
// Purpose:   Implementation of class wex::shell on_key
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/factory/defs.h>
#include <wex/factory/process.h>
#include <wex/stc/shell.h>

void wex::shell::on_key_back_delete(wxKeyEvent& event)
{
  if (GetCurrentPos() <= m_command_start_pos)
  {
    // Ignore, so do nothing.
  }
  else
  {
    // Allow.
    process_char(event.GetKeyCode());

    if (m_echo)
    {
      event.Skip();
    }
  }
}

void wex::shell::on_key_down(wxKeyEvent& event)
{
  if (!on_key_down_continue(event))
  {
    return;
  }

  switch (const auto key = event.GetKeyCode(); key)
  {
    case WXK_RETURN:
    case WXK_TAB:
      if (m_echo && process_char(key))
        event.Skip();
      break;

    // Up or down key pressed, and at the end of document (and auto_complete
    // active)
    case WXK_UP:
    case WXK_DOWN:
      on_key_up_down(event);
      break;

    case WXK_HOME:
      on_key_home();
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
      on_key_back_delete(event);
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
      on_key_down_others(event);
  }
}

bool wex::shell::on_key_down_continue(wxKeyEvent& event)
{
  if (!m_enabled)
  {
    if (get_vi().mode().is_insert())
    {
      DocumentEnd();
      get_vi().mode().escape();
    }
    if (
      GetCurrentPos() >= m_command_start_pos &&
      (m_process == nullptr || m_process->is_running()))
    {
      enable(true);
    }
    else
    {
      event.Skip();
      return false;
    }
  }
  else
  {
    if (
      config(_("stc.vi mode")).get(true) &&
      (GetCurrentPos() < m_command_start_pos))
    {
      enable(false);
      event.Skip();
      return false;
    }
  }

  return true;
}

void wex::shell::on_key_down_others(wxKeyEvent& event)
{
  const auto key  = event.GetKeyCode();
  bool       skip = true;

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
  // If we enter regular text and not already building a command, first
  // goto end.
  else if (
    event.GetModifiers() == wxMOD_NONE && key < WXK_START &&
    GetCurrentPos() < m_command_start_pos)
  {
    DocumentEnd();
  }

  m_commands_iterator = m_commands.end();

  if (m_echo && skip)
  {
    event.Skip();
  }
}

void wex::shell::on_key_home()
{
  Home();

  if (GetLine(get_current_line()).StartsWith(m_prompt))
  {
    GotoPos(GetCurrentPos() + m_prompt.length());
  }
}

void wex::shell::on_key_up_down(wxKeyEvent& event)
{
  if (GetCurrentPos() == GetTextLength() && !AutoCompActive())
  {
    show_command(event.GetKeyCode());
  }
  else
  {
    event.Skip();
  }
}
