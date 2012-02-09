////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/ex.h>
#include <wx/extension/defs.h>
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
  , m_IsActive(false)
  , m_SearchFlags(wxSTC_FIND_REGEXP | wxSTC_FIND_MATCHCASE)
  , m_MarkerSymbol(0, -1)
  , m_YankedLines(false)
{
  wxASSERT(m_Frame != NULL);
}

wxExEx::~wxExEx()
{
  if (m_Process != NULL)
  {
    delete m_Process;
  }
}

bool wxExEx::Command(const wxString& command)
{
  if (command.empty())
  {
    return false;
  }
  
  bool set_focus = true;
  bool result = true;

  if (command == ":$")
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
    Delete(1);
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
      set_focus = false;
      
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
      set_focus = false;
      
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
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_EDIT_READ);
    event.SetString(command.AfterFirst(' '));
    wxPostEvent(wxTheApp->GetTopWindow(), event);
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
    if (m_Process = NULL)
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
    m_Frame->HideExBar(set_focus);
    SetLastCommand(command);
    MacroRecord(command);
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
  
  wxString print;
    
  m_STC->SetSearchFlags(m_SearchFlags);

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(0);
  m_STC->SetTargetEnd(m_STC->GetTextLength());

  while (m_STC->SearchInTarget(pattern) > 0)
  {
    const int target_start = m_STC->GetTargetStart();

    if (target_start >= m_STC->GetTargetEnd())
    {
      break;
    }
    
    // TODO: Add more commands.
    if (command == "d")
    {
      m_STC->MarkTargetChange();
      
      const int begin = m_STC->PositionFromLine(m_STC->LineFromPosition(m_STC->GetTargetStart()));
      const int end = m_STC->PositionFromLine(m_STC->LineFromPosition(m_STC->GetTargetEnd()) + 1);
      
      m_STC->Remove(begin, end);
      m_STC->SetTargetStart(end);
      m_STC->SetTargetEnd(m_STC->GetTextLength());
    }
    else if (command == "p")
    {
      print += wxString::Format("%s:%d %s\n",
        m_STC->GetFileName().GetFullPath().c_str(),
        m_STC->LineFromPosition(m_STC->GetTargetStart()) + 1,
        m_STC->GetTextRange(m_STC->GetTargetStart(), m_STC->GetTargetEnd()));
      
      m_STC->SetTargetStart(m_STC->GetTargetEnd());
      m_STC->SetTargetEnd(m_STC->GetTextLength());
    }
    else if (command == "s")
    {
      m_STC->MarkTargetChange();
      const int target_start = m_STC->GetTargetStart();
      const int length = m_STC->ReplaceTargetRE(replacement); // always RE!
      m_STC->SetTargetStart(target_start + length);
      m_STC->SetTargetEnd(m_STC->GetTextLength());
    }
    else
    {
      return false;
    }
  }
  
  if (command == "p")
  {
    m_Frame->OpenFile("print", print);
  }

  m_STC->EndUndoAction();

  return true;
}

bool wxExEx::CommandRange(const wxString& command)
{
  // [address] m destination
  // [address] s [/pattern/replacement/] [options] [count]
  wxStringTokenizer tkz(command, "dmsyw><");
  
  if (!tkz.HasMoreTokens())
  {
    return false;
  }

  const wxString address = tkz.GetNextToken();
  const wxChar cmd = tkz.GetLastDelimiter();

  wxString begin_address;
  wxString end_address;
    
  if (address == ".")
  {
    begin_address = address;
    end_address = address;
  }
  else if (address == "%")
  {
    begin_address = "1";
    end_address = "$";
  }
  else if (address == "*")
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
    begin_address = address.BeforeFirst(',');
    end_address = address.AfterFirst(',');
  }
  
  if (begin_address.empty() || end_address.empty())
  {
    return false;
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
    return Move(begin_address, end_address, tkz.GetString());
    break;
    
  case 's':
    {
    wxStringTokenizer next(tkz.GetString(), "/");

    if (!next.HasMoreTokens())
    {
      return false;
    }

    next.GetNextToken(); // skip empty token
    const wxString pattern = next.GetNextToken();
    const wxString replacement = next.GetNextToken();
  
    return Substitute(begin_address, end_address, pattern, replacement);
    }
    break;
    
  case 'y':
    return Yank(begin_address, end_address);
    break;
    
  case 'w':
    return Write(begin_address, end_address, tkz.GetString());
    break;
    
  case '>':
    return Indent(begin_address, end_address, true);
    break;
  case '<':
    return Indent(begin_address, end_address, false);
    break;
    
  default:
    wxFAIL;
    return false;
  }
}

bool wxExEx::CommandSet(const wxString& command)
{
  // e.g. set ts=4
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
    
  return false;
}

void wxExEx::Delete(int lines) const
{
  if (m_STC->GetReadOnly() || m_STC->HexMode())
  {
    return;
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
    m_STC->Cut();
  }

  if (lines >= 2)
  {
    m_Frame->ShowExMessage(
      wxString::Format(_("%d fewer lines"), 
      linecount - m_STC->GetLineCount()));
  }
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
    MarkerDelete(begin_address.GetChar(1));
  }

  if (end_address.StartsWith("'"))
  {
    MarkerDelete(end_address.GetChar(1));
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
  if (m_STC->GetReadOnly())
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
    wxSingleChoiceDialog dialog(m_STC,
      _("Input") + ":", 
      _("Select Macro"),
      m_Macros.Get());
      
    if (dialog.ShowModal() != wxID_OK)
    {
      return false;
    }
    
    choice = dialog.GetStringSelection();
  }
  
  wxExSTC* stc = m_STC;
  
  const bool ok = m_Macros.Playback(this, choice, repeat);
  
  m_STC = stc;
  
  return ok;
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
    wxLogError(wxString::Format("Could not add marker: %c to line: %d",
      marker, lin));
    return false;  
  }
    
  m_Markers[marker] = id;
  
  return true;
}  

bool wxExEx::MarkerDelete(const wxUniChar& marker)
{
#ifdef wxExUSE_CPP0X	
  const auto it = m_Markers.find(marker);
#else
  const std::map<wxUniChar, int>::iterator it = m_Markers.find(marker);
#endif

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
#ifdef wxExUSE_CPP0X	
  const auto it = m_Markers.find(marker);
#else
  const std::map<wxUniChar, int>::const_iterator it = m_Markers.find(marker);
#endif	

  if (it != m_Markers.end())
  {
    return m_STC->MarkerLineFromHandle(it->second);
  }
  else
  {
    wxBell();
    return -1;
  }
}

bool wxExEx::Move(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& destination)
{
  if (m_STC->GetReadOnly())
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
    MarkerDelete(begin_address.GetChar(1));
  }

  if (end_address.StartsWith("'"))
  {
    MarkerDelete(end_address.GetChar(1));
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
  // First test on dot and ;, these should never be the last command,
  // even if always were true.
  if (command == "." || command == ";")
  {
    return;
  }
  
  if (
     always || 
    (command.size() > 2 && !command.StartsWith("\t")))
  {
    m_LastCommand = command;
  }
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
  m_STC->SetSelectionEnd(m_STC->PositionFromLine(end_line));

  return true;
}

bool wxExEx::Substitute(
  const wxString& begin_address, 
  const wxString& end_address, 
  const wxString& patt,
  const wxString& repl)
{
  if (m_STC->GetReadOnly())
  {
    return false;
  }

  if (m_STC->HexMode())
  {
    wxLogStatus(_("Not allowed in hex mode"));
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
  
  wxString replacement(repl);
  m_Replacement = replacement; 
  
  m_STC->SetSearchFlags(m_SearchFlags);

  int nr_replacements = 0;

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(begin_line - 1));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(MarkerLine('$')));

  while (m_STC->SearchInTarget(pattern) > 0)
  {
    const int target_start = m_STC->GetTargetStart();

    if (target_start >= m_STC->GetTargetEnd())
    {
      break;
    }
    
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
    
    m_STC->MarkTargetChange();
    
    const int length = m_STC->ReplaceTargetRE(replacement); // always RE!
    
    m_STC->SetTargetStart(target_start + length);
    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(MarkerLine('$')));

    nr_replacements++;
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
    
    const int line = MarkerLine(
      filtered_address.AfterFirst('\'').GetChar(0)) + 1;
    
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
    m_STC->CopyRange(start, end);
  }
  else
  {
    m_STC->CopyRange(start, m_STC->GetLastPosition());
  }
  
  m_YankedLines = true;

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

  m_STC->CopyRange(start, end);
  
  m_YankedLines = true;

  const int lines = end_line - begin_line + 1;
  
  if (lines >= 2)
  {
    m_Frame->ShowExMessage(wxString::Format(_("%d lines yanked"), lines));
  }

  return true;
}

#endif // wxUSE_GUI
