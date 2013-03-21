////////////////////////////////////////////////////////////////////////////////
// Name:      shell.cpp
// Purpose:   Implementation of class wxExSTCShell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
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
#include <wx/extension/process.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

BEGIN_EVENT_TABLE(wxExSTCShell, wxExSTC)
  EVT_CHAR(wxExSTCShell::OnChar)
  EVT_KEY_DOWN(wxExSTCShell::OnKey)
  EVT_MIDDLE_UP(wxExSTCShell::OnMouse)
  EVT_STC_CHARADDED(wxID_ANY, wxExSTCShell::OnStyledText)
  EVT_STC_DO_DROP(wxID_ANY, wxExSTCShell::OnStyledText)  
  EVT_STC_START_DRAG(wxID_ANY, wxExSTCShell::OnStyledText)
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
  , m_Process(NULL)
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
      ++it)
    {
      values += *it + m_CommandsInConfigDelimiter;
      items++;
    }

    wxConfigBase::Get()->Write("Shell", values);
  }
}

void wxExSTCShell::AppendText(const wxString& text)
{
  wxExSTC::AppendText(text);
  DocumentEnd();
  EnsureCaretVisible();
  m_CommandStartPosition = GetCurrentPos();
  EmptyUndoBuffer();
  
#ifdef DEBUG
  wxLogMessage("AppendText::" + GetText() + "::");
#endif
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

void wxExSTCShell::Expand()
{
  // We might have commands:
  // 1) ls -l s
  // -> should build list with e.g. sample and src.
  // path:   s
  // word:   s
  // subdir: empty
  // 2) ls -l src/
  // path:   src/
  // subdir: src
  // word:   empty
  // -> should build list with all files in src
  // 3) ls -l src/vi
  // -> should build list with files in src starting with vi
  // path:   src/vi
  // subdir: src
  // word:   vi
  const wxString path(m_Command.AfterLast(' '));
  const wxString word(path.AfterLast(wxFileName::GetPathSeparator()));
  
  wxString expansion;
  
  if (AutoCompActive())
  {
    const int index = AutoCompGetCurrent();
    
    if (index >=0 && index < m_AutoCompleteList.GetCount())
    {
      expansion = m_AutoCompleteList[index].Mid(word.length());
    }
    
    AutoCompCancel();
  }
  else
  {
    wxString subdir = path.BeforeLast(wxFileName::GetPathSeparator());
    
    if (!subdir.empty())
    {
      subdir = wxFileName::GetPathSeparator() + subdir;
    }
    
    wxDir dir(wxGetCwd() + subdir);
    wxString filename;
  
    if (dir.GetFirst(&filename, word + "*"))
    {
      wxString next;
    
      if (!dir.GetNext(&next))
      {
        expansion = filename.Mid(word.length());
        
        if (wxDirExists(dir.GetNameWithSep() + filename))
        {
          expansion += wxFileName::GetPathSeparator();
        }
      }
      else
      {
        // Fill the autocomplete list and show it
        // (when user selected something,
        // we come back at Expand at entry).
        m_AutoCompleteList.Clear();
        m_AutoCompleteList.Add(filename);
        m_AutoCompleteList.Add(next);
      
        while (dir.GetNext(&next))
        {
          m_AutoCompleteList.Add(next);
        }
      
        wxString list;
      
        for (int i = 0; i < m_AutoCompleteList.GetCount(); i++)
        {
          list += m_AutoCompleteList[i] + " ";
        }
      
        list.Trim(); // skip last whitespace separator
      
        AutoCompShow(word.length(), list);
      }
    }
  }
  
  // If we have an expansion.
  if (!expansion.empty())
  {
    m_Command += expansion;
    
    // We cannot use our AppendText, as command start pos
    // should not be changed.
    wxExSTC::AppendText(expansion);
    DocumentEnd();
  }
  
#ifdef DEBUG
  wxLogMessage("Expand::" + GetText() + "::");
#endif
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

// No longer used, for the moment.
void wxExSTCShell::OnCommand(wxCommandEvent& command)
{
  if (!m_Enabled)
  {
    command.Skip();
    return;
  }
  
  switch (command.GetId())
  {
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

  if (key == WXK_RETURN || key == WXK_TAB)
  {
    ProcessChar(key);
  }
  // Up or down key pressed, and at the end of document (and autocomplete active)
  else if ((key == WXK_UP || key == WXK_DOWN) &&
            GetCurrentPos() == GetTextLength() &&
           !AutoCompActive())
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
  // Ctrl-C pressed and no text selected (otherwise copy), also used to stop processing.
  else if (
    event.GetModifiers() == wxMOD_CONTROL && 
   ( key == 'Q' || 
    (key == 'C' && GetSelectedText().empty())))
  {
    if (m_Process != NULL)
    {
      m_Process->Command(ID_SHELL_COMMAND_STOP, wxEmptyString);
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
  // Shift-Insert key pressed, used for pasting.
  else if (
    event.GetModifiers() == wxMOD_SHIFT && 
    key == WXK_INSERT) 
  {
    Paste();
  }
  // Middle mouse button, to paste, though actually OnMouse is used.
  else if (key == WXK_MBUTTON)
  {
    Paste();
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
      ProcessChar(key);
      if (m_Echo) event.Skip();
    }
  }
  else if (key == WXK_ESCAPE)
  {
    if (AutoCompActive())
    {
      AutoCompCancel();
    }
    else
    {
      event.Skip();
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

void wxExSTCShell::OnMouse(wxMouseEvent& event)
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

void wxExSTCShell::OnStyledText(wxStyledTextEvent& event)
{
  if (!m_Enabled)
  {
    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_STC_CHARADDED)
  {
    // do nothing, keep event from sent to wxExSTC.
  }
  // Currently no drag/drop, though we might be able to
  // drag/drop copy to command line.
  else if (event.GetEventType() == wxEVT_STC_START_DRAG)
  {
#if wxUSE_DRAG_AND_DROP
    event.SetDragAllowMove(false);
#endif    
    
    event.Skip();
  }
  else if (event.GetEventType() == wxEVT_STC_DO_DROP)
  {
#if wxUSE_DRAG_AND_DROP
      event.SetDragResult(wxDragNone);
#endif    
    
    event.Skip();
  }
}

void wxExSTCShell::Paste()
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
  
  wxExSTC::Paste();
  
  m_Command += wxExClipboardGet();  
  
#ifdef DEBUG
  wxLogMessage("Paste::" + GetText() + "::");
#endif
}

void wxExSTCShell::ProcessChar(int key)
{
  // No need to check m_Enabled, already done by calling this method.
  
  if (key == WXK_RETURN)
  {
    if (AutoCompActive())
    {
      Expand();
    }
    else if (m_Command.empty())
    {
      if (m_Process != NULL)
      {
        m_Process->Command(ID_SHELL_COMMAND, m_Command);
      }
      else
      {
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND);
        event.SetString(m_Command);
        wxPostEvent(GetParent(), event);
      }
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
          if (m_Process != NULL)
          {
            m_Process->Command(ID_SHELL_COMMAND, m_Command);
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
          Prompt(GetEOL() + m_Command + ": " + _("event not found"));
        }
      }
      // Other command, send to parent or process.
      else
      {
        KeepCommand();
        
        if (m_Process != NULL)
        {
          m_Process->Command(ID_SHELL_COMMAND, m_Command);
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

    m_CommandsIterator = m_Commands.end();
  }
  else if (key == WXK_BACK || key == WXK_DELETE)
  {
    // Delete the key at current position.
    const int offset = (key == WXK_BACK ? 1: 0);
    const int index = GetCurrentPos() - m_CommandStartPosition - offset;
    
    if (index >= 0 && index < m_Command.length() && m_Command.length() > 0)
    {
      m_Command.erase(index, 1);
    }
  }
  else if (key == WXK_TAB)
  {
    Expand();
  }
  else
  {
    // Insert the key at current position.
    const int index = GetCurrentPos() - m_CommandStartPosition;
    
    if (
      GetCurrentPos() < GetLength() && 
      index >= 0 && index < m_Command.size())
    {
      m_Command.insert(index, wxChar(key));
    }
    else
    {
      m_Command += wxChar(key);
    }
  }
  
#ifdef DEBUG
  wxLogMessage("ProcessChar::" + GetText() + "::");
#endif
}

bool wxExSTCShell::Prompt(const wxString& text, bool add_eol)
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

  if (!m_Prompt.empty())
  {
    appended = true;
    
    if (GetTextLength() > 0 && add_eol)
    {
      AppendText(GetEOL());
    }
    
    AppendText(m_Prompt);
  }
  
  if (!appended)
  {
    DocumentEnd();
    EmptyUndoBuffer();
    m_CommandStartPosition = GetCurrentPos();
  }
  
#ifdef DEBUG
  wxLogMessage("Prompt::" + GetText() + "::");
#endif

  return true;
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
      ++it)
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
      ++it)
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

void wxExSTCShell::SetProcess(wxExProcess* process)
{
  m_Process = process;
}

bool wxExSTCShell::SetPrompt(const wxString& prompt, bool do_prompt) 
{
  if (!m_Enabled)
  {
    return false;
  }
  
  m_Prompt = prompt;
  
  if (do_prompt) 
  {
    Prompt();
  }
  
  return true;
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
  else
  {
    Undo();
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
    ++it)
  {
    const wxString command = *it;

    AppendText(wxString::Format("\n%d %s",
      command_no++,
      command.c_str()));
  }
  
#ifdef DEBUG
  wxLogMessage("ShowHistory::" + GetText() + "::");
#endif
}

void wxExSTCShell::Undo()
{
  if (CanUndo())
  {
    wxExSTC::Undo();
    m_Command.clear();
  }
  
#ifdef DEBUG
  wxLogMessage("Undo::" + GetText() + "::");
#endif
}
#endif // wxUSE_GUI
