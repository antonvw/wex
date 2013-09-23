////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/textfile.h>
#include <wx/extension/ex.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

wxExViMacros wxExEx::m_Macros;
wxString wxExEx::m_LastCommand;
  
wxExEx::wxExEx(wxExSTC* stc)
  : m_STC(stc)
  , m_Process(NULL)
  , m_Frame(wxDynamicCast(wxTheApp->GetTopWindow(), wxExManagedFrame))
  , m_IsActive(true)
  , m_SearchFlags(wxSTC_FIND_REGEXP)
  , m_MarkerSymbol(0, -1)
  , m_FindIndicator(0, 0)
{
  wxASSERT(m_Frame != NULL);
  
  if (wxExFindReplaceData::Get()->MatchCase())
  {
    m_SearchFlags = m_SearchFlags | wxSTC_FIND_MATCHCASE;
  }
}

wxExEx::~wxExEx()
{
  if (m_Process != NULL)
  {
    delete m_Process;
  }
}

void wxExEx::AddText(const wxString& text)
{
  if (!m_Register.empty())
  {
    m_Macros.SetRegister(m_Register, text);
  }
  else
  {
    m_STC->AddText(text);
  }
}

bool wxExEx::Command(const wxString& command)
{
  if (!m_IsActive || !command.StartsWith(":"))
  {
    return false;
  }
  
  bool result = true;

  if (command == ":" || command == ":'<,'>")
  {
    m_Frame->GetExCommand(this, command);
    return true;
  }
  else if (command == ":$")
  {
    m_STC->DocumentEnd();
  }
  else if (command == ":close")
  {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_CLOSE);
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command == ":d")
  {
    return Delete(1);
  }
  else if (command.StartsWith(":e"))
  {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_OPEN);
    
    if (command.Contains(" "))
    {
      event.SetString(command.AfterFirst(' '));
    }
    
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command.StartsWith(":g"))
  {
    result = CommandGlobal(command.AfterFirst('g'));
  }
  else if (command == ":n")
  {
    wxExSTC* stc = m_Frame->ExecExCommand(ID_EDIT_NEXT);
    
    if (stc != NULL)
    {
      if (m_Macros.IsPlayback())
      {
        m_STC = stc;
      }
    }
    else
    {
      result = false;
    }
  }
  else if (command == ":prev")
  {
    wxExSTC* stc = m_Frame->ExecExCommand(ID_EDIT_PREVIOUS);
    
    if (stc != NULL)
    {
      if (m_Macros.IsPlayback())
      {
        m_STC = stc;
      }
    }
    else
    {
      result = false;
    }
  }
  else if (command == ":q")
  {
    wxCloseEvent event(wxEVT_CLOSE_WINDOW);
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command == ":q!")
  {
    wxCloseEvent event(wxEVT_CLOSE_WINDOW);
    event.SetCanVeto(false); 
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command.StartsWith(":r"))
  {
    wxString arg(command.AfterFirst(' '));
    arg.Trim(false); // from left
    
    if (arg.empty())
    {
      return false;
    }
    
    if (arg.StartsWith("!"))
    {
      if (m_Process == NULL)
      {
        m_Process = new wxExProcess;
      }
    
      if (m_Process->Execute(arg.AfterFirst('!'), wxEXEC_SYNC))
      {
        m_STC->AddText(m_Process->GetOutput());
      }
    }
    else
    {
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_EDIT_READ);
      event.SetString(arg);
      wxPostEvent(wxTheApp->GetTopWindow(), event);
    }
  }
  // e.g. set ts=4
  else if (command.StartsWith(":set "))
  {
    result = CommandSet(command.Mid(5));
  }
  else if (command.StartsWith(":w"))
  {
    if (command.Contains(" "))
    {
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVEAS);
      event.SetString(command.AfterFirst(' '));
      wxPostEvent(wxTheApp->GetTopWindow(), event);
    }
    else
    {
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE);
      wxPostEvent(wxTheApp->GetTopWindow(), event);
    }
  }
  else if (command == ":x")
  {
    wxPostEvent(wxTheApp->GetTopWindow(), 
      wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE));
      
    wxPostEvent(wxTheApp->GetTopWindow(), 
      wxCloseEvent(wxEVT_CLOSE_WINDOW));
  }
  else if (command == ":y")
  {
    Yank(1);
  }
  else if (command.Last() == '=')
  {
    const int no = ToLineNumber(command.AfterFirst(':').BeforeLast('='));
    
    if (no == 0)
    {
      return false;
    }
    
    m_Frame->ShowExMessage(wxString::Format("%d", no));
    return true;
  }
  else if (command.StartsWith(":!"))
  {
    if (m_Process == NULL)
    {
      m_Process = new wxExProcess;
    }
    
    m_Process->Execute(command.AfterFirst('!'));
  }
  else if (command.AfterFirst(':').IsNumber())
  {
    m_STC->GotoLineAndSelect(atoi(command.AfterFirst(':').c_str()));
  }
  else
  {
    result = CommandRange(command.AfterFirst(':'));
  }

  if (result)
  {  
    SetLastCommand(command);
    m_Macros.Record(command);
  }
  else
  {
    wxBell();
  }
  
  return result;
}

bool wxExEx::CommandGlobal(const wxString& search)
{
  wxStringTokenizer next(search, "/");

  if (!next.HasMoreTokens())
  {
    return false;
  }

  next.GetNextToken(); // skip empty token
  const wxString pattern = next.GetNextToken();
  const wxString command = next.GetNextToken();
  const wxString skip = next.GetNextToken();
  const wxString replacement = next.GetNextToken();
  const int linecount = m_STC->GetLineCount();
  
  int nr_replacements = 0;

  wxString print;
  
  m_STC->SetIndicatorCurrent(m_FindIndicator.GetNo());
  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
    
  m_STC->SetSearchFlags(m_SearchFlags);

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(0);
  m_STC->SetTargetEnd(m_STC->GetTextLength());

  while (m_STC->SearchInTarget(pattern) != -1)
  {
    if (command == "d")
    {
      const int begin = m_STC->PositionFromLine(
        m_STC->LineFromPosition(m_STC->GetTargetStart()));
      const int end = m_STC->PositionFromLine(
        m_STC->LineFromPosition(m_STC->GetTargetEnd()) + 1);
      
      m_STC->Remove(begin, end);
      m_STC->SetTargetStart(begin);
      m_STC->SetTargetEnd(m_STC->GetTextLength());
    }
    else if (command == "p")
    {
      print += m_STC->GetLine(m_STC->LineFromPosition(m_STC->GetTargetStart()));
        
      m_STC->SetIndicator(
        m_FindIndicator, m_STC->GetTargetStart(), m_STC->GetTargetEnd());
      
      m_STC->SetTargetStart(m_STC->GetTargetEnd());
      m_STC->SetTargetEnd(m_STC->GetTextLength());
    }
    else if (command == "s")
    {
      m_STC->ReplaceTargetRE(replacement); // always RE!
      m_STC->SetTargetStart(m_STC->GetTargetEnd());
      m_STC->SetTargetEnd(m_STC->GetTextLength());
        
      nr_replacements++;
      
      if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
      {
        break;
      }
    }
    else
    {
      m_STC->EndUndoAction();
      return false;
    }
  }
  
  if (command == "d")
  {
    if (linecount - m_STC->GetLineCount() > 0)
    {
      m_Frame->ShowExMessage(
        wxString::Format(_("%d fewer lines"), 
        linecount - m_STC->GetLineCount()));
    }
  }
  else if (command == "p")
  {
    m_Frame->OpenFile("print", print);
  }
  else if (command == 's')
  {
    m_Frame->ShowExMessage(wxString::Format(_("Replaced: %d occurrences of: %s"),
      nr_replacements, pattern.c_str()));
  }

  m_STC->EndUndoAction();

  return true;
}

bool wxExEx::CommandRange(const wxString& command)
{
  wxString begin_address;
  wxString end_address;
  wxString text;
  wxChar cmd;

  if (command.StartsWith("'<,'>"))
  {
    begin_address = "'<";
    end_address = "'>";
    cmd = command.GetChar(5);
    text = command.Mid(6);
  }
  else
  {
    const wxString tokens("dmsyw><!");
      
    // We cannot yet handle a command containing tokens as markers.
    if (command.Contains("'"))
    {
      const wxString markerfirst = command.AfterFirst('\'');
      const wxString markerlast = command.AfterLast('\'');
    
      if (
        tokens.Contains(markerfirst.Left(1)) ||
        tokens.Contains(markerlast.Left(1)))
      {
        return false;
      }
    }
  
    // [range]m[destination]
    // [range]s/pattern/replacement/[options]
    wxStringTokenizer tkz(command, tokens);
  
    if (!tkz.HasMoreTokens())
    {
      return false;
    }

    const wxString range = tkz.GetNextToken();
    cmd = tkz.GetLastDelimiter();
  
    if (range == ".")
    {
      begin_address = range;
      end_address = range;
    }
    else if (range == "%")
    {
      begin_address = "1";
      end_address = "$";
    }
    else if (range == "*")
    {
      std::stringstream ss;
      ss << m_STC->GetFirstVisibleLine() + 1;
      begin_address = wxString(ss.str());
      std::stringstream tt;
      tt << m_STC->GetFirstVisibleLine() + m_STC->LinesOnScreen() + 1;
      end_address = wxString(tt.str());
    }
    else
    {
      begin_address = range.BeforeFirst(',');
      end_address = range.AfterFirst(',');
    }
  
    if (begin_address.empty() || end_address.empty())
    {
      return false;
    }
    
    text = tkz.GetString();
  }

  switch (cmd)
  {
  case 0: 
    return false; 
    break;
    
  case 'd':
    return Delete(begin_address, end_address);
    break;
    
  case 'm':
    return Move(begin_address, end_address, text);
    break;
    
  case 's':
    {
    // If there are escaped / chars in the text,
    // temporarily replace them to an unused char, so
    // we can use string tokenizer with / as separator.
    bool escaped = false;
    
    if (text.Contains("\\/"))
    {
      if (!text.Contains(wxChar(1)))
      {
        text.Replace("\\/", wxChar(1));
        escaped = true;
      }
      else
      {
        wxLogStatus("Cannot substitute, internal char exists");
        return false;
      }
    }
    
    wxStringTokenizer next(text, "/");

    if (!next.HasMoreTokens())
    {
      return false;
    }

    next.GetNextToken(); // skip empty token
    wxString pattern = next.GetNextToken();
    wxString replacement = next.GetNextToken();
    const wxString options = next.GetNextToken();
    
    // Restore a / for all occurrences of the special char.
    if (escaped)
    {  
      pattern.Replace(wxChar(1), "/");
      replacement.Replace(wxChar(1), "/");
    }
    
    return Substitute(
      begin_address, end_address, pattern, replacement, options);
    }
    break;
    
  case 'y':
    return Yank(begin_address, end_address);
    break;
    
  case 'w':
    return Write(begin_address, end_address, text);
    break;
    
  case '>':
    return Indent(begin_address, end_address, true);
    break;
  case '<':
    return Indent(begin_address, end_address, false);
    break;
    
  case '!':
    {
      // A filter command.
      // The address range is used as input for the command,
      // and the output of the command is replaces the address range.
      const int begin_line = ToLineNumber(begin_address);
      const int end_line = ToLineNumber(end_address);

      if (begin_line == 0 || end_line == 0 || end_line < begin_line)
      {
        return false;
      }
  
      wxExProcess process;
      char buffer[255];
      tmpnam(buffer);
      
      wxTextFile file(buffer);
      
      if (!file.Create())
      {
        return false;
      }
 
      wxString input;
      
      for (int i = begin_line - 1; i <= end_line - 1; i++)
      {
        file.AddLine(m_STC->GetLine(i).Trim());
      }
      
      file.Write();
        
      const wxString command = text;
      
      const bool ok = process.Execute(command + " " + buffer, wxEXEC_SYNC);
      
      remove(buffer);
      
      if (ok)
      {
        if (!process.HasStdError())
        {      
          m_STC->BeginUndoAction();
          
          Delete(begin_address, end_address);
          m_STC->AddText(process.GetOutput());
          
          m_STC->EndUndoAction();
          
          return true;
        }
        else
        {
          m_Frame->ShowExMessage(process.GetOutput());
        }
      }
      
      return false;
    }
    break;
    
  default:
    wxFAIL;
    return false;
  }
}

bool wxExEx::CommandSet(const wxString& command)
{
  const bool on = !command.EndsWith("!");
  
  // e.g.: set ts=4
  if (command.StartsWith("ts") || command.StartsWith("tabstop"))
  {
    const int val = atoi(command.AfterFirst('='));

    if (val > 0)
    {
      m_STC->SetTabWidth(val);
      wxConfigBase::Get()->Write(_("Tab width"), val);
      return true;
    }
  }
  else if (command.StartsWith("ic")) // ignore case
  {
    if (!on) m_SearchFlags |= wxSTC_FIND_MATCHCASE;
    else     m_SearchFlags &= ~wxSTC_FIND_MATCHCASE;
    
    wxExFindReplaceData::Get()->SetMatchCase(!on);
    return true;
  }
  else if (command.StartsWith("nu")) // number
  {
    m_STC->ShowLineNumbers(on);
    return true;
  }
  else if (command.StartsWith("li")) // list
  {
    m_STC->SetViewEOL(on);
    m_STC->SetViewWhiteSpace(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
    return true;
  }
  
  return false;
}

bool wxExEx::Delete(int lines)
{
  if (m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }
  
  const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
  const int start_pos = m_STC->PositionFromLine(line);
  const int end_pos = m_STC->PositionFromLine(line + lines);
  const int linecount = m_STC->GetLineCount();
    
  m_STC->SetSelectionStart(start_pos);

  if (end_pos != -1)
  {
    m_STC->SetSelectionEnd(end_pos);
  }
  else
  {
    m_STC->DocumentEndExtend();
  }

  if (m_STC->GetSelectedText().empty())
  {
    m_STC->DeleteBack();
  }
  else
  {
    if (!m_Register.empty())
    {
      m_Macros.SetRegister(m_Register, m_STC->GetSelectedText());
      m_STC->ReplaceSelection(wxEmptyString);
    }
    else
    {
      m_STC->Cut();
    }
  }

  if (lines >= 2)
  {
    m_Frame->ShowExMessage(
      wxString::Format(_("%d fewer lines"), 
      linecount - m_STC->GetLineCount()));
  }
  
  return true;
}

bool wxExEx::Delete(
  const wxString& begin_address, 
  const wxString& end_address)
{
  if (m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }
  
  if (!SetSelection(begin_address, end_address))
  {
    return false;
  }

  const int lines = wxExGetNumberOfLines(m_STC->GetSelectedText());
  
  m_STC->Cut();

  if (begin_address.StartsWith("'"))
  {
    if (begin_address.size() > 1)
    {
      MarkerDelete(begin_address.GetChar(1));
    }
  }

  if (end_address.StartsWith("'"))
  {
    if (end_address.size() > 1)
    {
      MarkerDelete(end_address.GetChar(1));
    }
  }

  if (lines >= 2)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d fewer lines"), lines));
  }

  return true;
}

bool wxExEx::Indent(
  const wxString& begin_address, 
  const wxString& end_address, 
  bool forward)
{
  if (m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }
  
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }

  m_STC->Indent(begin_line - 1, end_line - 1, forward);
  
  return true;
}

bool wxExEx::MacroPlayback(const wxString& macro, int repeat)
{
  if (!m_IsActive)
  {
    return false;
  }
  
  wxString choice(macro);
  
  if (choice.empty())
  {
    auto v(m_Macros.Get());
  
    if (v.empty())
    {
      return false;
    }
  
    wxArrayString macros;
    
    for (auto it = v.begin(); it != v.end(); ++it)
    {
      macros.Add(*it);
    }
    
    wxSingleChoiceDialog dialog(m_STC,
      _("Input") + ":", 
      _("Select Macro"),
      macros);
      
    const int index = macros.Index(m_Macros.GetMacro());
  
    if (index != wxNOT_FOUND)
    {
      dialog.SetSelection(index);
    }

    if (dialog.ShowModal() != wxID_OK)
    {
      return false;
    }
    
    choice = dialog.GetStringSelection();
  }
  
  wxExSTC* stc = m_STC;
  bool ok;
  
  if (m_Macros.IsRecordedMacro(choice))
  {
    ok = m_Macros.Playback(this, choice, repeat);
  }
  else
  {
    ok = m_Macros.Expand(this, choice);
  }
    
  m_STC = stc;
  
  if (ok)
  {
    m_Frame->StatusText(m_Macros.GetMacro(), "PaneMacro");
  }
  
  return ok;
}

void wxExEx::MacroRecord(const wxString& text)
{
  m_Macros.Record(text);
}

void wxExEx::MacroStartRecording(const wxString& macro)
{
  if (!m_IsActive)
  {
    return;
  }
  
  wxString choice(macro);
  
  if (choice.empty())
  {
    wxTextEntryDialog dlg(m_STC,
      _("Input") + ":",
      _("Enter Macro"),
      m_Macros.GetMacro());
  
    if (dlg.ShowModal() != wxID_OK)
    {
      return;
    }
    
    choice = dlg.GetValue();
  }
  
  m_Macros.StartRecording(choice);
}

bool wxExEx::MarkerAdd(const wxUniChar& marker, int line)
{
  MarkerDelete(marker);
  
  const int lin = (line == -1 ? m_STC->GetCurrentLine(): line);
  
  const int id = m_STC->MarkerAdd(
    lin, 
    m_MarkerSymbol.GetNo());
    
  if (id == -1)
  {
    wxLogError("Could not add marker: %c to line: %d",
      marker, lin);
    return false;  
  }
    
  m_Markers[marker] = id;
  
  return true;
}  

bool wxExEx::MarkerDelete(const wxUniChar& marker)
{
  const auto it = m_Markers.find(marker);

  if (it != m_Markers.end())
  {
    m_STC->MarkerDeleteHandle(it->second);
    m_Markers.erase(it);
    return true;
  }
  
  return false;
}

bool wxExEx::MarkerGoto(const wxUniChar& marker)
{
  const int line = MarkerLine(marker);
  
  if (line != -1)
  {
    m_STC->GotoLineAndSelect(line + 1);
    return true;
  }
  
  return false;
}

int wxExEx::MarkerLine(const wxUniChar& marker) const
{
  if (marker == '<')
  {
    return m_STC->LineFromPosition(m_STC->GetSelectionStart());
  }
  else if (marker == '>')
  {
    return m_STC->LineFromPosition(m_STC->GetSelectionEnd());
  }
  else
  {
    const auto it = m_Markers.find(marker);

    if (it != m_Markers.end())
    {
      return m_STC->MarkerLineFromHandle(it->second);
    }
    else
    {
      wxBell();
      wxLogStatus(_("Undefined marker: %c"), marker);
      return -1;
    }
  }
}

bool wxExEx::Move(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& destination)
{
  if (m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }

  const int dest_line = ToLineNumber(destination);

  if (dest_line == 0)
  {
    return false;
  }

  if (!SetSelection(begin_address, end_address))
  {
    return false;
  }

  if (begin_address.StartsWith("'"))
  {
    if (begin_address.size() > 1)
    {
      MarkerDelete(begin_address.GetChar(1));
    }
  }

  if (end_address.StartsWith("'"))
  {
    if (end_address.size() > 1)
    {
      MarkerDelete(end_address.GetChar(1));
    }
  }

  m_STC->BeginUndoAction();

  m_STC->Cut();
  m_STC->GotoLine(dest_line - 1);
  m_STC->Paste();

  m_STC->EndUndoAction();
  
  const int lines = wxExGetNumberOfLines(m_STC->GetSelectedText());
  if (lines >= 2)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d lines moved"), lines));
  }

  return true;
}

void wxExEx::SetLastCommand(
  const wxString& command,
  bool always)
{
  // First test on '.' and ';' these should never be the last command,
  // even if always were true.
  // Also, undo or placing a marker should not be a last command.
  if (
    command == "." || command == ";" || 
    command == "u" || command.Matches("m?"))
  {
    return;
  }
  
  if (
      always || 
    ( command.StartsWith(":") && command.size() > 2) ||
    (!command.StartsWith(":") && command.size() > 1 && !command.StartsWith("\t")))
  {
    m_LastCommand = command;
  }
}
 
void wxExEx::SetRegistersDelete(const wxString& value)
{
  for (int i = 9; i >= 2; i--)
  {
    m_Macros.SetRegister(
      wxString::Format("%d", i),
      m_Macros.GetRegister(wxString::Format("%d", i - 1)));
  }
  
  m_Macros.SetRegister("1", m_STC->GetSelectedText());
}
  
void wxExEx::SetRegisterYank(const wxString& value)
{
  m_Macros.SetRegister("0", value);
}

bool wxExEx::SetSelection(
  const wxString& begin_address, 
  const wxString& end_address) const
{
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }

  m_STC->SetSelectionStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetSelectionEnd(m_STC->GetLineEndPosition(end_line - 1));

  return true;
}

bool wxExEx::Substitute(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& patt,
  const wxString& repl,
  const wxString& options)
{
  if (m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return false;
  }
  
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }
  
  if (!MarkerAdd('$', end_line - 1))
  {
    return false;
  }

  const wxString pattern = (patt == "~" ? m_Replacement: patt);
  
  m_Replacement = repl; 
      
  int searchFlags = m_SearchFlags;
  
  if (options.Contains("i")) searchFlags &= ~wxSTC_FIND_MATCHCASE;
    
  m_STC->SetSearchFlags(searchFlags);

  int nr_replacements = 0;

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(MarkerLine('$')));

  int result = wxID_YES;
       
  while (m_STC->SearchInTarget(pattern) != -1 && result != wxID_CANCEL)
  {
    wxString replacement(repl);
    
    if (replacement.Contains("&"))
    {
      wxString target = m_STC->GetTextRange(
        m_STC->GetTargetStart(),
        m_STC->GetTargetEnd());
        
      if (replacement.StartsWith("\\L"))
      {
        target.MakeLower();
        replacement.Replace("\\L", wxEmptyString);
      }
      else if (replacement.StartsWith("\\U"))
      {
        target.MakeUpper();
        replacement.Replace("\\U", wxEmptyString);
      }
    
      replacement.Replace("&", target);
    }
    
    if (options.Contains("c"))
    {
      wxMessageDialog msgDialog(m_STC, 
        _("Replace") + " " + pattern + " " + _("with") + " " + replacement, 
        _("Replace"), 
        wxCANCEL | wxYES_NO);
      
      msgDialog.SetExtendedMessage(m_STC->GetLineText(
        m_STC->LineFromPosition(m_STC->GetTargetStart())));
      
      result = msgDialog.ShowModal();
        
      if (result == wxID_YES)
      {
        m_STC->ReplaceTargetRE(replacement); // always RE!
      }
    }
    else
    {
      m_STC->ReplaceTargetRE(replacement); // always RE!
    }
        
    if (result != wxID_CANCEL)
    {
      if (options.Contains("g"))
      {
        m_STC->SetTargetStart(m_STC->GetTargetEnd());
      }
      else
      {
        m_STC->SetTargetStart(
          m_STC->GetLineEndPosition(m_STC->LineFromPosition(
            m_STC->GetTargetEnd())));
      }
  
      m_STC->SetTargetEnd(m_STC->GetLineEndPosition(MarkerLine('$')));
    
      if (result == wxID_YES)
      {
        nr_replacements++;
      }
    
      if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
      {
        result = wxID_CANCEL;
      }
    }
  }
  
  m_STC->EndUndoAction();
  
  MarkerDelete('$');

  m_Frame->ShowExMessage(wxString::Format(_("Replaced: %d occurrences of: %s"),
    nr_replacements, pattern.c_str()));

  return true;
}

// Returns 0 and bells on error in address, otherwise the vi line number,
// so subtract 1 for stc line number.
int wxExEx::ToLineNumber(const wxString& address) const
{
  wxString filtered_address(wxExSkipWhiteSpace(address, ""));

  // Filter all markers.
  int markers = 0;

  while (filtered_address.Contains("'"))
  {
    const wxString oper = filtered_address.BeforeFirst('\'');
    
    int pos = filtered_address.Find('\'');
    int size = 2;
    
    const wxString marker = filtered_address.AfterFirst('\'');
    
    if (marker.empty())
    {
      return 0;
    }
    
    const int line = MarkerLine(marker.GetChar(0)) + 1;
    
    if (line == 0)
    {
      return 0;
    }

    if (oper == "-")
    {
      markers -= line;
      pos--;
      size++;
    }
    else if (oper == "+")
    {
      markers += line;
      pos--;
      size++;
    }
    else 
    {
      markers += line;
    }

    filtered_address.replace(pos, size, "");
  }

  int dot = 0;
  int stc_used = 0;

  if (filtered_address.Contains("."))
  {
    dot = m_STC->GetCurrentLine();
    filtered_address.Replace(".", "");
    stc_used = 1;
  }

  // Filter $.
  int dollar = 0;

  if (filtered_address.Contains("$"))
  {
    dollar = m_STC->GetLineCount();
    filtered_address.Replace("$", "");
    stc_used = 1;
  }

  // Now we should have a number.
  if (!filtered_address.IsNumber()) 
  {
    return 0;
  }

  // Convert this number.
  int i = 0;
  
  if (!filtered_address.empty())
  {
    if ((i = atoi(filtered_address.c_str())) == 0)
    {
      return 0;
    }
  }
  
  // Calculate the line.
  const int line_no = markers + dot + dollar + i + stc_used;
  
  // Limit the range of what is returned.
  if (line_no <= 0)
  {
    return 1;
  }
  else if (line_no > m_STC->GetLineCount())
  {
    return m_STC->GetLineCount();
  }  
  else
  {
    return line_no;
  }
}

bool wxExEx::Write(
  const wxString& begin_address, 
  const wxString& end_address,
  const wxString& filename) const
{
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0 || end_line < begin_line)
  {
    return false;
  }

  wxFile file(filename, wxFile::write);

  return 
    file.IsOpened() && 
    file.Write(m_STC->GetTextRange(
      m_STC->PositionFromLine(begin_line - 1), 
      m_STC->PositionFromLine(end_line)));
}

void wxExEx::Yank(int lines)
{
  const int line = m_STC->LineFromPosition(m_STC->GetCurrentPos());
  const int start = m_STC->PositionFromLine(line);
  const int end = m_STC->PositionFromLine(line + lines);

  if (end != -1)
  {
    SetRegisterYank(m_STC->GetTextRange(start, end));
  
    if (!m_Register.empty())
    {
      m_Macros.SetRegister(m_Register, m_STC->GetTextRange(start, end));
    }
    else
    {
      m_STC->CopyRange(start, end);
    }
  }
  else
  {
    SetRegisterYank(m_STC->GetTextRange(
      start, m_STC->GetLastPosition()));
      
    if (!m_Register.empty())
    {
      m_Macros.SetRegister(
        m_Register, 
        m_STC->GetTextRange(start, m_STC->GetLastPosition()));
    }
    else
    {  
      m_STC->CopyRange(start, m_STC->GetLastPosition());
    }
  }
  
  if (lines >= 2)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d lines yanked"), lines));
  }
}

bool wxExEx::Yank(
  const wxString& begin_address, 
  const wxString& end_address)
{
  const int begin_line = ToLineNumber(begin_address);
  const int end_line = ToLineNumber(end_address);

  if (begin_line == 0 || end_line == 0)
  {
    return false;
  }

  const int start = m_STC->PositionFromLine(begin_line - 1);
  const int end = m_STC->PositionFromLine(end_line);
  
  if (start == end)
  {
    return false;
  }

  m_STC->CopyRange(start, end);
  SetRegisterYank(m_STC->GetTextRange(start, end));
  
  const int lines = end_line - begin_line + 1;
  
  if (lines >= 2)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d lines yanked"), lines));
  }

  return true;
}

#endif // wxUSE_GUI
