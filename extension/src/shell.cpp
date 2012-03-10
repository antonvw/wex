////////////////////////////////////////////////////////////////////////////////
// Name:      shell.cpp
// Purpose:   Implementation of class wxExSTCShell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

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
  EVT_CHAR(wxExSTCShell::OnChar)
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
      STC_WIN_NO_INDICATOR,
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
  , m_Handler(parent)
  , m_Enabled(true)
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
  
  EnableShell(true);

  SetLexer(lexer);
}

wxExSTCShell::~wxExSTCShell()
{
  if (m_CommandsSaveInConfig > 0)
  {
    wxString values;
    int items = 0;

    for (
#ifdef wxExUSE_CPP0X	
      auto it = m_Commands.rbegin();
#else
      std::list < wxString >::reverse_iterator it = m_Commands.rbegin();
#endif	  
      it != m_Commands.rend() && items < m_CommandsSaveInConfig;
      it++)
    {
      values += *it + m_CommandsInConfigDelimiter;
      items++;
    }

    wxConfigBase::Get()->Write("Shell", values);
  }
}

void wxExSTCShell::EnableShell(bool enabled)
{
  m_Enabled = enabled;
  
  if (!m_Enabled)
  {
    // A disabled shell follows STC vi mode.
    GetVi().Use(wxConfigBase::Get()->ReadBool(_("vi mode"), true));
  }
  else
  {
    // An enabled shell does not use vi mode.
    GetVi().Use(false);
  }
}

const wxString wxExSTCShell::GetCommand() const
{
  if (!m_Commands.empty())
  {
    return m_Commands.back();
  }
  else
  {
    return wxEmptyString;
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
  if (!m_Enabled)
  {
    command.Skip();
    return;
  }
  
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

void wxExSTCShell::OnChar(wxKeyEvent& event)
{
  if (m_Enabled)
  {
    ProcessChar(event.GetKeyCode());
  }
  
  if (m_Echo)
  {
    event.Skip();
  }
}

void wxExSTCShell::OnKey(wxKeyEvent& event)
{
  if (!m_Enabled)
  {
    event.Skip();
    return;
  }
  
  const int key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    ProcessChar(key);
  }
  else if (key == WXK_BACK)
  {
    ProcessChar(key);
    if (m_Echo) event.Skip();
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
  else if (
    event.GetModifiers() == wxMOD_CONTROL && 
   (key == 'Q' || key == 'C'))
  {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND_STOP);
    wxPostEvent(m_Handler, event);
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
  if (!m_Enabled)
  {
    event.Skip();
    return;
  }
  
  // do nothing, keep event from sent to wxExSTC.
}

void wxExSTCShell::ProcessChar(int key)
{
  if (key == '\r')
  {
    if (m_Command.empty())
    {
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
      event.SetString(m_Command);
      wxPostEvent(m_Handler, event);
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
          wxPostEvent(m_Handler, event);
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
        wxPostEvent(m_Handler, event);
      }

      m_Command.clear();
    }

    m_CommandsIterator = m_Commands.end();
  }
  else if (key == WXK_BACK)
  {
    m_Command = m_Command.Truncate(m_Command.size() - 1);
  }
  else
  {
    m_Command += wxChar(key);
  }
}

void wxExSTCShell::Prompt(const wxString& text, bool add_eol)
{
  if (!m_Enabled)
  {
    return;
  }
  
  if (!text.empty())
  {
    AppendText(text);
    
    if (GetTextLength() > 0 && add_eol)
    {
      AppendText(GetEOL());
    }
  }

  if (!m_Prompt.empty())
  {
    AppendText(m_Prompt);
  }

  DocumentEnd();

  m_CommandStartPosition = GetCurrentPos();

  EmptyUndoBuffer();
}

bool wxExSTCShell::SetCommandFromHistory(const wxString& short_command)
{
  const int no_asked_for = atoi(short_command.c_str());

  if (no_asked_for > 0)
  {
    int no = 1;

    for (
#ifdef wxExUSE_CPP0X	
      auto it = m_Commands.begin();
#else
      std::list < wxString >::iterator it = m_Commands.begin();
#endif	  
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
#ifdef wxExUSE_CPP0X	
      auto it = m_Commands.rbegin();
#else
      std::list < wxString >::reverse_iterator it = m_Commands.rbegin();
#endif	  
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

  DocumentEnd();
}

void wxExSTCShell::ShowHistory()
{
  int command_no = 1;

  for (
#ifdef wxExUSE_CPP0X	
    auto it = m_Commands.begin();
#else
    std::list < wxString >::iterator it = m_Commands.begin();
#endif	
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
