////////////////////////////////////////////////////////////////////////////////
// Name:      shell.cpp
// Purpose:   Implementation of class wxExShell
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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

const char autoCompSep = 3;

#if wxUSE_GUI

wxExShell::wxExShell(
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
      wxEmptyString, // title, used for name
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
  , m_Process(nullptr)
  , m_Enabled(true)
{
  // Override defaults from config.
  SetEdgeMode(wxSTC_EDGE_NONE);
  ResetMargins(false); // do not reset divider margin
  UseAutoComplete(false); // we have our own autocomplete
  AutoCompSetSeparator(autoCompSep);

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
  
  Bind(wxEVT_CHAR, [=](wxKeyEvent& event) {
    if (m_Enabled)
    {
      ProcessChar(event.GetKeyCode());
    }
    
    if (m_Echo)
    {
      event.Skip();
    }});

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
      if (GetVi().ModeInsert())
      {
        DocumentEnd();
        GetVi().Command("\x1b"); // ESC, normal mode
      }
      if (GetCurrentPos() >= m_CommandStartPosition)
      {
        EnableShell(true);
      }
      else
      {
        event.Skip();
        return;
      }
    }
    else
    {
      if (wxConfigBase::Get()->ReadBool(_("vi mode"), true) && (GetCurrentPos() < m_CommandStartPosition))
      {
        EnableShell(false);
        event.Skip();
        return;
      }
    }

    bool skip = true;
    const int key = event.GetKeyCode();
    
    switch (key)
    {
      case WXK_RETURN:
      case WXK_TAB:
        if (m_Echo && ProcessChar(key)) event.Skip();
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
          ProcessChar(key);
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
        // Ctrl-V pressed, used for pasting.
        if (key == 'V' && event.GetModifiers() == wxMOD_CONTROL)
        {
          Paste();
        }
        // Ctrl-Q pressed, used to stop processing.
        // Ctrl-C pressed and no text selected (otherwise copy), also used to stop processing.
        else if (
          event.GetModifiers() == wxMOD_CONTROL && 
          (key == 'Q' || 
          (key == 'C' && GetSelectedText().empty())))
        {
          skip = false;
        
          if (m_Process != nullptr)
          {
            m_Process->Kill();
          }
          else
          {
            wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_SHELL_COMMAND_STOP);
            wxPostEvent(GetParent(), event);
          }
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
    // do nothing, keep event from sent to wxExSTC.
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

wxExShell::~wxExShell()
{
  if (m_CommandsSaveInConfig > 0)
  {
    wxString values;
    int items = 0;

    for (
      auto it = m_Commands.rbegin(); 
      it != m_Commands.rend() && items < m_CommandsSaveInConfig;
      ++it)
    {
      values += *it + m_CommandsInConfigDelimiter;
      items++;
    }

    wxConfigBase::Get()->Write("Shell", values);
  }
}

void wxExShell::AppendText(const wxString& text)
{
  const bool pos_at_end = (GetCurrentPos() >= GetTextLength());

  wxExSTC::AppendText(text);
  
  m_CommandStartPosition = GetTextLength();
  
  EmptyUndoBuffer();
  
  if (pos_at_end)
  {
    DocumentEnd();
    EnsureCaretVisible();
  }
}

void wxExShell::EnableShell(bool enabled)
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

void wxExShell::Expand()
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
    
    if (index >= 0 && index < (int)m_AutoCompleteList.size())
    {
      expansion = m_AutoCompleteList[index].Mid(word.length());
    }
    
    AutoCompCancel();
  }
  else
  {
    if (wxExAutoCompleteFileName(m_Command, m_AutoCompleteList))
    {
      if (m_AutoCompleteList.size() == 2)
      {
        expansion = m_AutoCompleteList[0];
      }
      else
      {
        wxString list;
        m_AutoCompleteList.erase(m_AutoCompleteList.begin());
      
        for (const auto& it : m_AutoCompleteList)
        {
          list += it + autoCompSep;
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
    
const wxString wxExShell::GetCommand() const
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

const wxString wxExShell::GetHistory() const
{
  return accumulate(m_Commands.begin(), m_Commands.end(), wxString());
}

void wxExShell::KeepCommand()
{
  // Prevent large commands, in case command end is not eol.
  if (m_CommandEnd != GetEOL())
  {
    m_Command = wxExSkipWhiteSpace(m_Command);
  }
  
  m_Commands.push_back(m_Command);
}

void wxExShell::Paste()
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

bool wxExShell::ProcessChar(int key)
{
  bool processed = false;
  
  // No need to check m_Enabled, already done by calling this method.
  switch (key)
  {
    case WXK_RETURN:
      if (AutoCompActive())
      {
        if (!m_AutoCompleteList.empty())
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
          m_Process->Command(m_Command);
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
            KeepCommand();
          
            if (m_Process != nullptr)
            {
              m_Process->Command(m_Command);
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
          
          if (m_Process != nullptr)
          {
            m_Process->Command(m_Command);
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
        ProcessCharDefault(key);
        processed = true;
      }

      m_CommandsIterator = m_Commands.end();
      break;
  
    case WXK_BACK:
    case WXK_DELETE:
      {
        // Delete the key at current position.
        const int offset = (key == WXK_BACK ? 1: 0);
        const int index = GetCurrentPos() - m_CommandStartPosition - offset;
        
        if (index >= 0 && index < (int)m_Command.length() && m_Command.length() > 0)
        {
          m_Command.erase(index, 1);
        }
      }
      break;
    
    case WXK_TAB:
      Expand();
      break;
      
    default:
      ProcessCharDefault(key);
      processed = true;
  }
  
#ifdef DEBUG
  wxLogMessage("ProcessChar::" + GetText() + "(" + m_Command + ")::");
#endif

  return processed;
}

void wxExShell::ProcessCharDefault(int key)
{
  // Insert the key at current position.
  const int index = GetCurrentPos() - m_CommandStartPosition;
  
  if (
    GetCurrentPos() < GetLength() && 
    index >= 0 && index < (int)m_Command.size())
  {
    m_Command.insert(index, wxChar(key));
  }
  else
  {
    m_Command += wxChar(key);
  }
}
  
bool wxExShell::Prompt(const wxString& text, bool add_eol)
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
  
  DocumentEnd();
  
  if (!appended)
  {
    EmptyUndoBuffer();
  }
  
  m_CommandStartPosition = GetCurrentPos();
  
#ifdef DEBUG
  wxLogMessage("Prompt::" + GetText() + "::");
#endif

  return true;
}

bool wxExShell::SetCommandFromHistory(const wxString& short_command)
{
  const int no_asked_for = atoi(short_command.c_str());

  if (no_asked_for > 0)
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

void wxExShell::SetProcess(wxExProcess* process)
{
  m_Process = process;
}

bool wxExShell::SetPrompt(const wxString& prompt, bool do_prompt) 
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

void wxExShell::ShowCommand(int key)
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

void wxExShell::ShowHistory()
{
  int command_no = 1;

  for (const auto& it : m_Commands)
  {
    AppendText(wxString::Format("\n%d %s", command_no++, it.c_str()));
  }
  
#ifdef DEBUG
  wxLogMessage("ShowHistory::" + GetText() + "::");
#endif
}

void wxExShell::Undo()
{
  if (CanUndo())
  {
    wxExSTC::Undo();
  }
  
  m_Command.clear();

#ifdef DEBUG
  wxLogMessage("Undo::" + GetText() + "::");
#endif
}
#endif // wxUSE_GUI
