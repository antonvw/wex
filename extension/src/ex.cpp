////////////////////////////////////////////////////////////////////////////////
// Name:      ex.cpp
// Purpose:   Implementation of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/tokenzr.h>
#include <wx/extension/ex.h>
#include <wx/extension/address.h>
#include <wx/extension/defs.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

wxExViMacros wxExEx::m_Macros;
std::string wxExEx::m_LastCommand;
  
wxExEx::wxExEx(wxExSTC* stc)
  : m_STC(stc)
  , m_Process(NULL)
  , m_Frame(wxDynamicCast(wxTheApp->GetTopWindow(), wxExManagedFrame))
  , m_IsActive(true)
  , m_SearchFlags(wxSTC_FIND_REGEXP)
  , m_MarkerSymbol(0, -1)
  , m_FindIndicator(0, 0)
  , m_Register(0)
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

void wxExEx::AddText(const std::string& text)
{
  if (m_Register)
  {
    m_Macros.SetRegister(m_Register, text);
  }
  else
  {
    m_STC->AddText(text);
  }
}

bool wxExEx::Command(const std::string& command)
{
  if (!m_IsActive || command.empty() || command.front() != ':')
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
    return wxExAddressRange(this, 1).Delete();
  }
  else if (command.compare(0, 2, ":e") == 0)
  {
    wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_OPEN);
    
    if (command.find(" ") != std::string::npos)
    {
      event.SetString(command.substr(command.find(" ")));
    }
    
    wxPostEvent(wxTheApp->GetTopWindow(), event);
  }
  else if (command.compare(0, 2, ":g") == 0)
  {
    result = CommandGlobal(wxString(command).AfterFirst('g'));
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
  else if (command.compare(0, 2, ":r") == 0)
  {
    wxString arg(wxString(command).AfterFirst(' '));
    arg.Trim(false); // from left
    
    if (arg.empty())
    {
      return false;
    }
    
    if (arg.StartsWith("!"))
    {
      if (m_Process == NULL)
      {
        m_Process = new wxExProcess(m_STC);
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
  else if (command.compare(0, 4, ":set") == 0)
  {
    result = CommandSet(wxString(command.substr(4)).Trim(false));
  }
  else if (command.compare(0, 7, ":syntax") == 0)
  {
    const bool on = wxString(command).EndsWith("on");
  
    if (on)
    {
      wxExLexers::Get()->RestoreTheme();
      m_STC->SetLexer(m_STC->GetFileName().GetLexer().GetDisplayLexer());
    }
    else
    {
      m_STC->ResetLexer();
      wxExLexers::Get()->SetThemeNone();
    }

    m_Frame->StatusText(wxExLexers::Get()->GetTheme(), "PaneTheme");
  }
  else if (command.compare(0, 2, ":w") == 0)
  {
    if (command.find(" ") != std::string::npos)
    {
      wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVEAS);
      event.SetString(command.substr(command.find(" ")));
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
    wxExAddressRange(this, 1).Yank();
  }
  else if (command.back() == '=')
  {
    const wxExAddress address(this, wxString(command).AfterFirst(':').BeforeLast('='));
    const int no = address.ToLine();
    
    if (no == 0)
    {
      return false;
    }
    
    m_Frame->ShowExMessage(wxString::Format("%d", no));
    return true;
  }
  else if (command.compare(0, 2, ":!") == 0)
  {
    if (m_Process == NULL)
    {
      m_Process = new wxExProcess(m_STC);
    }
    
    m_Process->Execute(
      command.substr(2),
      wxEXEC_ASYNC,
      m_STC->GetFileName().GetPath());
  }
  else if (CommandRange(command.substr(1)))
  {
    // do nothing
  }
  else if (wxExAddress(this, command.substr(1)).ToLine() > 0)
  {
    m_STC->GotoLineAndSelect(wxExAddress(this, command.substr(1)).ToLine());
  }
  else
  {
    result = false;
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
  const int command =
    (next.HasMoreTokens() ? (int)next.GetNextToken().GetChar(0): (int)' ');
  const wxString skip = next.GetNextToken();
  const wxString replacement = next.GetNextToken();
  const int linecount = m_STC->GetLineCount();
  
  int hits = 0;

  wxString print;
  
  m_STC->SetIndicatorCurrent(m_FindIndicator.GetNo());
  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  if (pattern.empty())
  {
    if (!replacement.empty())
    {
      wxLogStatus("Cannot replace, pattern is empty");
      return false;
    }
    
    // Silently cleared indicators.
    return true;  
  }
    
  m_STC->SetSearchFlags(m_SearchFlags);

  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(0);
  m_STC->SetTargetEnd(m_STC->GetTextLength());

  while (m_STC->SearchInTarget(pattern) != -1)
  {
    switch (command)
    {
      case 'd':
      {
        const int begin = m_STC->PositionFromLine(
          m_STC->LineFromPosition(m_STC->GetTargetStart()));
        const int end = m_STC->PositionFromLine(
          m_STC->LineFromPosition(m_STC->GetTargetEnd()) + 1);
        
        m_STC->Remove(begin, end);
        m_STC->SetTargetStart(begin);
        m_STC->SetTargetEnd(m_STC->GetTextLength());
      }
      break;
  
    case 'p':
      print += m_STC->GetLine(m_STC->LineFromPosition(m_STC->GetTargetStart()));
    case ' ':
      m_STC->SetIndicator(
        m_FindIndicator, m_STC->GetTargetStart(), m_STC->GetTargetEnd());
      
      m_STC->SetTargetStart(m_STC->GetTargetEnd());
      m_STC->SetTargetEnd(m_STC->GetTextLength());
      hits++;
      break;
      
    case 's':
      m_STC->ReplaceTargetRE(replacement); // always RE!
      m_STC->SetTargetStart(m_STC->GetTargetEnd());
      m_STC->SetTargetEnd(m_STC->GetTextLength());
      hits++;
      break;
      
    default:
      m_STC->EndUndoAction();
      return false;
    }
  
    if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
    {
      break;
    }
  }
  
  switch (command)
  {
    case ' ':
      m_Frame->ShowExMessage(wxString::Format(_("Found: %d occurrences of: %s"),
        hits, pattern.c_str()));
      break;
    case 'd': 
      if (linecount - m_STC->GetLineCount() > 0)
      {
        m_Frame->ShowExMessage(
          wxString::Format(_("%d fewer lines"), 
          linecount - m_STC->GetLineCount()));
      }
      break;
    case 'p':
      if (!print.empty())
      {
        m_Frame->OpenFile("print", print);
      }
      break;
    case 's':
      m_Frame->ShowExMessage(wxString::Format(_("Replaced: %d occurrences of: %s"),
        hits, pattern.c_str()));
      break;
  }

  m_STC->EndUndoAction();

  return true;
}

bool wxExEx::CommandRange(const wxString& command)
{
  wxString rest;
  wxString range_str;  
  wxChar cmd;

  if (command.compare(0, 5, "'<,'>") == 0)
  {
    if (m_STC->GetSelectedText().empty())
    {
      return false;
    }

    range_str = "'<,'>";
    cmd = command.GetChar(5);
    rest = command.Mid(6);
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
  
    wxStringTokenizer tkz(command, tokens);
  
    if (!tkz.HasMoreTokens())
    {
      return false;
    }

    range_str = tkz.GetNextToken();
    cmd = tkz.GetLastDelimiter();
    rest = tkz.GetString();
  }

  wxExAddressRange range(this, range_str);
  
  switch (cmd)
  {
  case 0: return false; break;
  case 'd': return range.Delete(); break;
  case 'm': return range.Move(wxExAddress(this, rest)); break;
  case 'y': return range.Yank(); break;
  case 'w': return range.Write(rest); break;
  case '>': return range.Indent(true); break;
  case '<': return range.Indent(false); break;
  case '!': return range.Filter(rest); break;
  case 's': return range.Substitute(rest); break;
  default:
    wxLogStatus("Unknown range command: %c", cmd);
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
  else if (command.StartsWith("li")) // list
  {
    m_STC->SetViewEOL(on);
    m_STC->SetViewWhiteSpace(on ? wxSTC_WS_VISIBLEALWAYS: wxSTC_WS_INVISIBLE);
    return true;
  }
  else if (command.StartsWith("nu")) // number
  {
    m_STC->ShowLineNumbers(on);
    return true;
  }
  else if (command.StartsWith("sy")) // syntax
  {
    if (on) m_STC->SetLexer(command.AfterFirst('='));
    else    m_STC->ResetLexer();
    return true;
  }
  
  return false;
}

const std::string wxExEx::GetRegisterText() 
{
  return m_Register ? 
    GetMacros().GetRegister(m_Register):
    GetMacros().GetRegister('0');
}
  
const std::string wxExEx::GetSelectedText() const
{
  if (m_STC->GetSelectedText().empty())
  {
    std::string none;
    return none;
  }

  const wxCharBuffer b(m_STC->GetSelectedTextRaw());
  return std::string(b.data(), b.length() - 1);
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
    const auto& v(m_Macros.Get());
    
    if (v.empty())
    {
      return false;
    }
  
    wxArrayString macros;
    macros.resize(v.size());
    copy(v.begin(), v.end(), macros.begin());
    
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
  bool ok = true;
  
  if (m_Macros.IsRecordedMacro(choice))
  {
    ok = m_Macros.Playback(this, choice, repeat);
  }
  else
  {
    for (int i = 0; i < repeat && ok; i++)
    {
      if (!m_Macros.Expand(this, choice))
      {
        ok = false;
      }
    }
  }
    
  m_STC = stc;
  
  if (ok)
  {
    m_Frame->StatusText(m_Macros.GetMacro(), "PaneMacro");
  }
  
  return ok;
}

void wxExEx::MacroRecord(const std::string& text)
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
  
    wxTextValidator validator(wxFILTER_ALPHANUMERIC);
    dlg.SetTextValidator(validator);
  
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
    if (m_STC->GetSelectedText().empty())
    {
      return -1;
    }
    
    return m_STC->LineFromPosition(m_STC->GetSelectionStart());
  }
  else if (marker == '>')
  {
    if (m_STC->GetSelectedText().empty())
    {
      return -1;
    }
  
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

void wxExEx::SetLastCommand(
  const std::string& command,
  bool always)
{
  // First test on '.' and ';' these should never be the last command,
  // even if always were true.
  // Also, undo or placing a marker should not be a last command.
  if (
    command == "." || command == ";" || 
    command == "u" || wxString(command).Matches("m?"))
  {
    return;
  }
  
  if (
      always || 
      command == "~" || 
    ( command.size() > 2 && command.front() == ':') ||
    ( command.size() > 1 && command.front() != ':' && command.front() != '\t'))
  {
    m_LastCommand = command;
  }
}
 
void wxExEx::SetRegistersDelete(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  for (int i = 9; i >= 2; i--)
  {
    const std::string value(m_Macros.GetRegister(wxUniChar(48 + i - 1)));
    
    if (!value.empty())
    {
      m_Macros.SetRegister(wxUniChar(48 + i), value);
    }
  }
  
  m_Macros.SetRegister('1', value);
}
  
void wxExEx::SetRegisterYank(const std::string& value) const
{
  if (value.empty())
  {
    return;
  }
  
  m_Macros.SetRegister('0', value);
}

#endif // wxUSE_GUI
