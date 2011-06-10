/******************************************************************************\
* File:          shell.cpp
* Purpose:       Implementation of class wxExSTCShell
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <numeric>
#include <functional>
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/shell.h>
#include <wx/extension/defs.h> // for ID_SHELL_COMMAND

#if wxUSE_GUI

BEGIN_EVENT_TABLE(wxExSTCShell, wxExSTC)
  EVT_KEY_DOWN(wxExSTCShell::OnKey)
  EVT_MENU(wxID_PASTE, wxExSTCShell::OnCommand)
  EVT_STC_CHARADDED(wxID_ANY, wxExSTCShell::OnStyledText)
END_EVENT_TABLE()

wxExSTCShell::wxExSTCShell(
  wxWindow* parent,
  const wxString& prompt,
  const wxString& command_end,
  bool echo,
  int commands_save_in_config,
  const wxString& lexer,
  long menu_flags,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style)
  : wxExSTC(
      parent, 
      wxEmptyString,
      0,
      wxEmptyString, // title
      menu_flags, 
      id, 
      pos, 
      size, 
      style)
  , m_Command(wxEmptyString)
  , m_CommandEnd((command_end == wxEmptyString ? GetEOL(): command_end))
  , m_CommandStartPosition(0)
  , m_Echo(echo)
  // take a char that is not likely to appear inside commands
  , m_CommandsInConfigDelimiter(wxUniChar(0x03))
  , m_CommandsSaveInConfig(commands_save_in_config)
  , m_Prompt(prompt)
{
  // Override defaults from config.
  SetEdgeMode(wxSTC_EDGE_NONE);
  ResetMargins(false); // do not reset divider margin
  SetName("SHELL");

  // Start with a prompt.
  Prompt();

  if (m_CommandsSaveInConfig > 0)
  {
    // Get all previous commands.
    wxStringTokenizer tkz(wxConfigBase::Get()->Read("Shell"),
      m_CommandsInConfigDelimiter);

    while (tkz.HasMoreTokens())
    {
      const wxString val = tkz.GetNextToken();
      m_Commands.push_front(val);
    }
  }

  // Take care that m_CommandsIterator is valid.
  m_CommandsIterator = m_Commands.end();

  SetLexer(lexer);
}

wxExSTCShell::~wxExSTCShell()
{
  if (m_CommandsSaveInConfig > 0)
  {
    wxString values;
    int items = 0;

    for (
      auto it = m_Commands.rbegin();
      it != m_Commands.rend() && items < m_CommandsSaveInConfig;
      it++)
    {
      values += *it + m_CommandsInConfigDelimiter;
      items++;
    }

    wxConfigBase::Get()->Write("Shell", values);
  }
}

const wxString wxExSTCShell::GetHistory() const
{
  return accumulate(m_Commands.begin(), m_Commands.end(), wxString());
}

void wxExSTCShell::KeepCommand()
{
  m_Commands.remove(m_Command);
  m_Commands.push_back(m_Command);
}

void wxExSTCShell::OnCommand(wxCommandEvent& command)
{
  switch (command.GetId())
  {
    case wxID_PASTE:
      // Take care that we cannot paste somewhere inside.
      if (GetCurrentPos() < m_CommandStartPosition)
      {
        DocumentEnd();
      }

      Paste();
      break;

    default: 
      wxFAIL;
      break;
  }
}

void wxExSTCShell::OnKey(wxKeyEvent& event)
{
  const auto key = event.GetKeyCode();

  // Enter key pressed, we might have entered a command.
  if (key == WXK_RETURN)
  {
    // First get the command.
    SetTargetStart(GetTextLength());
    SetTargetEnd(0);
    SetSearchFlags(wxSTC_FIND_REGEXP);

    if (SearchInTarget("^" + m_Prompt + ".*") != -1)
    {
      m_Command = GetText().substr(
        GetTargetStart() + m_Prompt.length(),
        GetTextLength() - 1);
      m_Command.Trim();
    }

    if (m_Command.empty())
    {
      Prompt();
    }
    else if (
      m_CommandEnd == GetEOL() ||
      m_Command.EndsWith(m_CommandEnd))
    {
      // We have a command.
      EmptyUndoBuffer();

      // History command.
      if (m_Command == wxString("history") +
         (m_CommandEnd == GetEOL() ? wxString(wxEmptyString): m_CommandEnd))
      {
        KeepCommand();
        ShowHistory();
        Prompt();
      }
      // !.. command, get it from history.
      else if (m_Command.StartsWith("!"))
      {
        if (SetCommandFromHistory(m_Command.substr(1)))
        {
          AppendText(GetEOL() + m_Command);

          // We don't keep the command, so commands are not rearranged and
          // repeatingly calling !5 always gives the same command, just as bash does.

          wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
          event.SetString(m_Command);
          wxPostEvent(GetParent(), event);
        }
        else
        {
          Prompt(GetEOL() + m_Command + ": " + _("event not found"));
        }
      }
      // Other command, send to parent.
      else
      {
        KeepCommand();
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
        event.SetString(m_Command);
        wxPostEvent(GetParent(), event);
      }

      m_Command.clear();
    }
    else
    {
      if (m_Echo) event.Skip();
    }

    m_CommandsIterator = m_Commands.end();
  }
  // Up or down key pressed, and at the end of document.
  else if ((key == WXK_UP || key == WXK_DOWN) &&
            GetCurrentPos() == GetTextLength())
  {
    ShowCommand(key);
  }
  // Home key pressed.
  else if (key == WXK_HOME)
  {
    Home();

    const wxString line = GetLine(GetCurrentLine());

    if (line.StartsWith(m_Prompt))
    {
      GotoPos(GetCurrentPos() + m_Prompt.length());
    }
  }
  // Ctrl-Q pressed, used to stop processing.
  else if (event.GetModifiers() == wxMOD_CONTROL && key == 'Q')
  {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND_STOP);
    wxPostEvent(GetParent(), event);
  }
  // Ctrl-V pressed, used for pasting as well.
  else if (event.GetModifiers() == wxMOD_CONTROL && key == 'V')
  {
    if (GetCurrentPos() < m_CommandStartPosition) DocumentEnd();
    if (m_Echo) event.Skip();
  }
  // Backspace or delete key pressed.
  else if (key == WXK_BACK || key == WXK_DELETE)
  {
    if (GetCurrentPos() <= m_CommandStartPosition)
    {
      // Ignore, so do nothing.
    }
    else
    {
      // Allow.
      if (m_Echo) event.Skip();
    }
  }
  // The rest.
  else
  {
    // If we enter regular text and not already building a command, first goto end.
    if (event.GetModifiers() == wxMOD_NONE &&
        key < WXK_START &&
        GetCurrentPos() < m_CommandStartPosition)
    {
      DocumentEnd();
    }

    m_CommandsIterator = m_Commands.end();

    if (m_Echo) event.Skip();
  }
}

void wxExSTCShell::OnStyledText(wxStyledTextEvent& event)
{
  // do nothing, keep event from sent to wxExSTC.
}

void wxExSTCShell::Prompt(const wxString& text, bool add_eol)
{
  if (!text.empty())
  {
    AppendText(text);
  }

  if (GetTextLength() > 0 && add_eol)
  {
    AppendText(GetEOL());
  }

  AppendText(m_Prompt);

  DocumentEnd();

  m_CommandStartPosition = GetCurrentPos();

  EmptyUndoBuffer();
}

bool wxExSTCShell::SetCommandFromHistory(const wxString& short_command)
{
  const auto no_asked_for = atoi(short_command.c_str());

  if (no_asked_for > 0)
  {
    int no = 1;

    for (
      auto it = m_Commands.begin();
      it != m_Commands.end();
      it++)
    {
      if (no == no_asked_for)
      {
        m_Command = *it;
        return true;
      }

      no++;
    }
  }
  else
  {
    wxString short_command_check;

    if (m_CommandEnd == GetEOL())
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
      it++)
    {
      const wxString command = *it;

      if (command.StartsWith(short_command_check))
      {
        m_Command = command;
        return true;
      }
    }
  }

  return false;
}

void wxExSTCShell::ShowCommand(int key)
{
  SetTargetStart(GetTextLength());
  SetTargetEnd(0);
  SetSearchFlags(wxSTC_FIND_REGEXP);

  if (SearchInTarget("^" + m_Prompt + ".*") != -1)
  {
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
      ReplaceTarget(m_Prompt + *m_CommandsIterator);
    }
    else
    {
      ReplaceTarget(m_Prompt);
    }

    DocumentEnd();
  }
}

void wxExSTCShell::ShowHistory()
{
  int command_no = 1;

  for (
    auto it = m_Commands.begin();
    it != m_Commands.end();
    it++)
  {
    const wxString command = *it;

    AppendText(wxString::Format("\n%d %s",
      command_no++,
      command.c_str()));
  }
}

#endif // wxUSE_GUI
