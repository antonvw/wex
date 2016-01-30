////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/tokenzr.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/indicator.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vimacros.h>

#if wxUSE_GUI

class GlobalEnv
{
public:
  GlobalEnv(wxExEx* ex, const wxString& commands)
  : m_Ex(ex)
  , m_Changes(0)
  , m_FindIndicator(0, 0)
  {
    m_Ex->GetSTC()->SetIndicatorCurrent(m_FindIndicator.GetNo());
    m_Ex->GetSTC()->SetSearchFlags(m_Ex->GetSearchFlags());
    m_Ex->GetSTC()->BeginUndoAction();
    
    wxStringTokenizer tkz(commands, "|");
    
    while (tkz.HasMoreTokens())
    {
      const std::string cmd(tkz.GetNextToken().ToStdString());
      
      if (!cmd.empty())
      {
        // Prevent recursive global.
        if (cmd[0] != 'g' && cmd[0] != 'v')
        {
          if (cmd[0] == 'd' || cmd[0] == 'm')
          {
            m_Changes++;
          }
          
          m_Commands.emplace_back(cmd);
        }
      }
    }
  }
  
 ~GlobalEnv()
  {
    m_Ex->GetSTC()->EndUndoAction();
    m_Ex->MarkerDelete('%');
  }
  
  int Changes() const {return m_Changes;};
  
  bool Commands() const {return !m_Commands.empty();};
  
  bool ForEach(int line) const
  {
    if (!Commands())
    {
      m_Ex->GetSTC()->SetIndicator(m_FindIndicator, 
        m_Ex->GetSTC()->GetTargetStart(), m_Ex->GetSTC()->GetTargetEnd());
    }
    else
    {
      for (const auto& it : m_Commands)
      {
        const std::string cmd(":" + std::to_string(line + 1) + it);

        if (!m_Ex->Command(cmd))
        {
          m_Ex->GetFrame()->ShowExMessage(wxString::Format("%s failed", cmd.c_str()));
          return false;
        }
      }
    }
    
    return true;
  }

  bool ForEach(int start, int& end, int& hits) const
  {
    if (start < end)
    {
      int i = start; 
      
      while (i < end && i < m_Ex->GetSTC()->GetLineCount() - 1)
      {
        if (Commands())
        {
          if (!ForEach(i)) return false;
        }
        else
        {
          m_Ex->GetSTC()->SetIndicator(
            m_FindIndicator, 
            m_Ex->GetSTC()->PositionFromLine(i), 
            m_Ex->GetSTC()->GetLineEndPosition(i));
        }
        
        if (m_Changes == 0) 
        {
          i++;
        }
        else 
        {
          end -= m_Changes;
        }
        
        hits++;
      }
    }
    else
    {
      end++;
    }
    
    return true;
  }        
private:
  const wxExIndicator m_FindIndicator;
  std::vector<std::string> m_Commands;
  int m_Changes;
  wxExEx* m_Ex;
};

wxString wxExAddressRange::m_Pattern;
wxString wxExAddressRange::m_Replacement;
wxExProcess* wxExAddressRange::m_Process = nullptr;

wxExAddressRange::wxExAddressRange(wxExEx* ex, int lines)
  : m_Begin(ex)
  , m_End(ex)
  , m_Ex(ex)
  , m_STC(ex->GetSTC())
{
  if (lines > 0) 
  {
    Set(m_Begin, m_End, lines);
  }
  else if (lines < 0)
  {
    Set(m_End, m_Begin, lines);
  }
}

wxExAddressRange::wxExAddressRange(wxExEx* ex, const wxString& range)
  : m_Begin(ex)
  , m_End(ex)
  , m_Ex(ex)
  , m_STC(ex->GetSTC())
{
  if (range == "%")
  {
    Set("1", "$");
  }
  else if (range == "*")
  {
    Set(
      m_STC->GetFirstVisibleLine() + 1, 
      m_STC->GetFirstVisibleLine() + m_STC->LinesOnScreen() + 1);
  }
  else if (range.Contains(","))
  {
    Set(range.BeforeFirst(','), range.AfterFirst(','));
  }
  else
  {
    Set(range, range);
  }
}

wxExAddressRange::~wxExAddressRange()
{
}
  
const wxString wxExAddressRange::BuildReplacement(const wxString& text) const
{
  if (!text.Contains("&") && !text.Contains("\0"))
  {
    return text;
  }

  wxString target(m_STC->GetTextRange(
    m_STC->GetTargetStart(), m_STC->GetTargetEnd()));
    
  wxString replacement;
  bool backslash = false;
    
  for (size_t i = 0; i < text.length(); i++)
  {
    switch ((int)text.GetChar(i))
    {
      case '&': 
        if (!backslash) 
          replacement << target; 
        else
          replacement << text[i];
        backslash = false; 
        break;
        
      case '0': 
        if (backslash) 
          replacement << target; 
        else
          replacement << text[i];
        backslash = false; 
        break;
        
      case 'L': 
        if (backslash) 
          target.MakeLower();
        else
          replacement << text[i];
        backslash = false; 
        break;
        
      case 'U': 
        if (backslash) 
          target.MakeUpper();
        else
          replacement << text[i];
        backslash = false; 
        break;
        
      case '\\': 
        if (backslash) 
          replacement << text[i];
        backslash = !backslash; 
        break;
        
      default:
        replacement << text[i];
        backslash = false; 
    }
  }

  return replacement;
}
  
bool wxExAddressRange::Change(const wxString& command) const
{
  if (!Delete())
  {
    return false;
  }
  
  m_Ex->GetSTC()->AddText(command);
  
  return true;
}
  
int wxExAddressRange::Confirm(
  const wxString& pattern, 
  const wxString& replacement, 
  const wxExIndicator& indicator)
{
  wxMessageDialog msgDialog(m_STC, 
    _("Replace") + " " + pattern + " " + _("with") + " " + replacement, 
    _("Replace"), 
    wxCANCEL | wxYES_NO);
    
  const int line = m_STC->LineFromPosition(m_STC->GetTargetStart());
  
  msgDialog.SetExtendedMessage(wxString::Format("Line %d: %s", 
    line + 1, m_STC->GetLineText(line).c_str()));
    
  m_STC->GotoLine(line);
  m_STC->EnsureVisible(line);
  m_STC->SetIndicator(
    indicator, m_STC->GetTargetStart(), m_STC->GetTargetEnd());
  
  return msgDialog.ShowModal();
}

bool wxExAddressRange::Copy(const wxExAddress& destination) const
{
  const int dest_line = destination.GetLine();

  if (m_STC->GetReadOnly() || m_STC->HexMode() || !IsOk() ||
     dest_line == 0 || 
    (dest_line >= m_Begin.GetLine() && dest_line <= m_End.GetLine()))
  {
    return false;
  }

  m_STC->BeginUndoAction();

  if (Yank())
  {
    m_STC->GotoLine(dest_line - 1);
    m_Ex->AddText(m_Ex->GetRegisterText());
  }

  m_STC->EndUndoAction();
  
  const int lines = wxExGetNumberOfLines(m_Ex->GetRegisterText());

  if (lines >= 2)
  {
    m_Ex->GetFrame()->ShowExMessage(
      wxString::Format(_("%d lines copied"), lines - 1));
  }

  return true;
}
  
bool wxExAddressRange::Delete(bool show_message) const
{
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !SetSelection())
  {
    return false;
  }

  m_Ex->Cut(show_message);
  
  m_Begin.MarkerDelete();
  m_End.MarkerDelete();

  return true;
}

bool wxExAddressRange::Escape(const wxString& command)
{
  if (m_Begin.empty() && m_End.empty())
  {
    if (m_Process == nullptr)
    {
      m_Process = new wxExProcess();
    }
  
    const bool ok = m_Process->Execute(
      command,
      wxEXEC_ASYNC,
      m_STC->GetFileName().GetPath());
    
    if (ok)
    {
      m_Ex->GetFrame()->ShowPane("PROCESS");
      wxExProcess::GetShell()->SetFocus();
    }
    
    return ok;
  }
  
  if (!IsOk())
  {
    return false;
  }
  
  const wxString filename("__TMPFILE__");
  
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !Write(filename))
  {
    return false;
  }

  wxExProcess process;
  
  const bool ok = process.Execute(command + " " + filename, wxEXEC_SYNC);
  
  if (remove(filename) != 0)
  {
    wxLogStatus("Could not remove file");
  }
  
  if (ok)
  {
    if (!process.HasStdError())
    {      
      m_STC->BeginUndoAction();

      if (Delete(false))
      {
        m_STC->AddText(process.GetOutput());
        m_STC->AddText(m_STC->GetEOL());
      }
      
      m_STC->EndUndoAction();
      
      return true;
    }
    else
    {
      m_Ex->GetFrame()->ShowExMessage(process.GetOutput());
    }
  }
  
  return false;
}

bool wxExAddressRange::Global(const wxString& text, bool inverse) const
{
  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  wxStringTokenizer next(text, "/");

  if (next.CountTokens() <= 1)
  {
    return true;
  }

  next.GetNextToken(); // skip empty token
  const wxString pattern = next.GetNextToken();
  std::string rest;
  
  if (next.HasMoreTokens())
  {
    int command = 0;
    const wxString token(next.GetNextToken());
    command = token.GetChar(0);
    wxString arg(token.Mid(1));
    
    if (next.HasMoreTokens())
    {
      wxString subpattern = next.GetNextToken();
      
      if (subpattern.empty())
      {
        subpattern = pattern;
      }
      
      arg += "/" + subpattern + "/" + next.GetString();
    }
    
    rest = std::string(1, command) + arg;
  }

  if (pattern.empty())
  {
    if (!rest.empty())
    {
      wxLogStatus("Pattern is empty");
      return false;
    }
    
    return true;  
  }
  
  const GlobalEnv g(m_Ex, rest);
  m_Ex->MarkerAdd('%', m_End.GetLine() - 1);
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Begin.GetLine() - 1));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('%')));
  
  const bool infinite = (g.Changes() > 0 && rest != "$" && rest != "1");
  int hits = 0;
  int start = 0;
  
  while (m_STC->SearchInTarget(pattern) != -1)
  {
    int match = m_STC->LineFromPosition(m_STC->GetTargetStart());
    
    if (!inverse)
    {
      if (!g.ForEach(match)) return false;
      hits++;
    }
    else
    {
      if (!g.ForEach(start, match, hits)) return false;
      start = match + 1;
    }
        
    if (hits > 50 && infinite)
    {
      m_Ex->GetFrame()->ShowExMessage(wxString::Format(
        "possible infinite loop at %d", match));
      return false;
    }
    
    m_STC->SetTargetStart(g.Changes() > 0 ? m_STC->PositionFromLine(match): m_STC->GetTargetEnd());
    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('%')));
  
    if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
    {
      break;
    }
  }
  
  if (inverse)
  {
    int match = m_STC->GetLineCount();
    if (!g.ForEach(start, match, hits)) return false;
  }
  
  if (hits > 0)
  {
    if (g.Commands())
      m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("Executed: %d commands"), hits));
    else
      m_Ex->GetFrame()->ShowExMessage(wxString::Format(_("Found: %d matches"), hits));
  }
  
  return true;
}

bool wxExAddressRange::Indent(bool forward) const
{
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !IsOk() || !SetSelection())
  {
    return false;
  }
  
  m_STC->BeginUndoAction();
  m_STC->SendMsg(forward ? wxSTC_CMD_TAB: wxSTC_CMD_BACKTAB);
  m_STC->EndUndoAction();
  
  return true;
}

bool wxExAddressRange::IsOk() const
{
  if (
    m_Begin.GetLine() <= 0 || m_End.GetLine() <= 0 || 
    m_Begin.GetLine() > m_End.GetLine())
  {
    return false;
  }

  return true;
}

bool wxExAddressRange::Join() const
{
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !IsOk())
  {
    return false;
  }
  
  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Begin.GetLine() - 1));
  m_STC->SetTargetEnd(m_STC->PositionFromLine(m_End.GetLine()));
  m_STC->LinesJoin();
  m_STC->EndUndoAction();
  
  return true;
}
  
bool wxExAddressRange::Move(const wxExAddress& destination) const
{
  const int dest_line = destination.GetLine();

  if (m_STC->GetReadOnly() || m_STC->HexMode() || !IsOk() ||
     dest_line == 0 || 
    (dest_line >= m_Begin.GetLine() && dest_line <= m_End.GetLine()))
  {
    return false;
  }

  m_STC->BeginUndoAction();

  if (Delete(false))
  {
    m_STC->GotoLine(dest_line - 1);
    m_Ex->AddText(m_Ex->GetRegisterText());
  }

  m_STC->EndUndoAction();
  
  const int lines = wxExGetNumberOfLines(m_Ex->GetRegisterText());

  if (lines >= 2)
  {
    m_Ex->GetFrame()->ShowExMessage(
      wxString::Format(_("%d lines moved"), lines - 1));
  }

  return true;
}

bool wxExAddressRange::Parse(
  const wxString& command_org, 
  wxString& pattern, wxString& replacement, wxString& options) const
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use string tokenizer with / as separator.
  bool escaped = false;
  
  wxString command(command_org);
  
  if (!command.Contains("\\\\/") && command.Contains("\\/"))
  {
    if (!command.Contains(wxChar(1)))
    {
      command.Replace("\\/", wxChar(1));
      escaped = true;
    }
    else
    {
      wxLogStatus("Internal char exists");
      return false;
    }
  }
  
  wxStringTokenizer next(command, "/");

  if (!next.HasMoreTokens())
  {
    wxLogStatus("Missing slash");
    return false;
  }

  next.GetNextToken(); // skip empty token
  pattern = next.GetNextToken();
  replacement = next.GetNextToken();
  options = next.GetNextToken();
  
  // Restore a / for all occurrences of the special char.
  if (escaped)
  {  
    pattern.Replace(wxChar(1), "/");
    replacement.Replace(wxChar(1), "/");
  }
  
  return true;
}
    
bool wxExAddressRange::Print(const wxString& flags) const
{
  if (!IsOk() || !m_Begin.Flags(flags))
  {
    return false;
  }
  
  wxString text;
  
  for (int i = m_Begin.GetLine() - 1; i < m_End.GetLine(); i++)
  {
    text += (flags.Contains("#") ? wxString::Format("%6d ", i + 1): "") + 
      m_STC->GetLine(i);
  }
    
  m_Ex->GetFrame()->PrintEx(m_Ex, text);
  
  return true;
}
  
void wxExAddressRange::Set(wxExAddress& begin, wxExAddress& end, int lines)
{
  begin.SetLine(m_STC->LineFromPosition(m_STC->GetCurrentPos()) + 1);
  end.SetLine(begin.GetLine() + lines - 1);
}

bool wxExAddressRange::SetSelection() const
{
  if (!m_STC->GetSelectedText().empty())
  {
    return true;
  }
  else if (!IsOk())
  {
    return false;
  }
  
  m_STC->SetSelection(
    m_STC->PositionFromLine(m_Begin.GetLine() - 1),
    m_STC->PositionFromLine(m_End.GetLine()));

  return true;
}

bool wxExAddressRange::Sort(const wxString& parameters) const
{
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !SetSelection())
  {
    return false;
  }
  
  size_t sort_type = STRING_SORT_ASCENDING;
  size_t pos = 0;
  size_t len = std::string::npos;

  if (m_STC->SelectionIsRectangle())
  {
    pos = m_STC->GetColumn(m_STC->GetSelectionStart());
    len = m_STC->GetColumn(m_STC->GetSelectionEnd() - pos);
  }

  if (!parameters.empty())
  {
    if (  (parameters[0] == '0') ||
         (!parameters.StartsWith("u") && 
          !parameters.StartsWith("r") && 
          !isdigit(parameters[0])))
    {
      return false;
    }
    
    sort_type |= (parameters.Contains("r") ? STRING_SORT_DESCENDING: 0);
    sort_type |= (parameters.Contains("u") ? STRING_SORT_UNIQUE: 0);
    
    if (isdigit(parameters[0]))
    {
      pos = (atoi(parameters) > 0 ? atoi(parameters) - 1: 0);
      
      if (parameters.Contains(","))
      {
        len = atoi(parameters.AfterFirst(',')) - pos + 1;
      }
    }
  }

  return wxExSortSelection(m_STC, sort_type, pos, len);
}
  
bool wxExAddressRange::Substitute(const wxString& text, const char cmd)
{
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !IsOk())
  {
    return false;
  }
  
  wxString pattern;
  wxString repl;
  wxString options;
  
  switch (cmd)
  {
    case 's':
      if (!Parse(text, pattern, repl, options))
      {
        return false;
      }
      break;
    case '&':
      repl = m_Replacement;
      pattern = m_Pattern;
      options = text;
      break;
    case '~':
      repl = m_Replacement;
      pattern = m_Pattern;
      options = text;
      break;
    default:
      return false;
  }
    
  if (!options.empty())
  {
    wxString filter(options);
    filter.Replace("c", "");
    filter.Replace("g", "");
    filter.Replace("i", "");
    
    if (!filter.empty())
    {
      wxLogStatus("Unsupported flags: " + filter);
      return false;
    }
  }
  
  if (pattern.empty())
  {
    wxLogStatus("Pattern is empty");
    return false;
  }

  int searchFlags = m_Ex->GetSearchFlags();
  if (options.Contains("i")) searchFlags &= ~wxSTC_FIND_MATCHCASE;
    
  if ((searchFlags & wxSTC_FIND_REGEXP) && 
    pattern.size() == 2 && pattern.Last() == '*' && repl.empty())
  {
    wxLogStatus("Replacement leads to infinite loop");
    return false;
  }
       
  if (!m_Ex->MarkerAdd('#', m_Begin.GetLine() - 1))
  {
    return false;
  }

  int corrected = 0;
  int end_line = m_End.GetLine() - 1;
  
  if (!m_STC->GetSelectedText().empty())
  {
    if (m_STC->GetLineSelEndPosition(end_line) == m_STC->PositionFromLine(end_line))
    {
      end_line--;
      corrected = 1;
    }
  }
  
  if (!m_Ex->MarkerAdd('$', end_line))
  {
    return false;
  }

  wxExIndicator indicator(0, 0);

  m_Pattern = pattern;
  m_Replacement = repl; 
  
  m_STC->SetIndicatorCurrent(indicator.GetNo());
  m_STC->SetSearchFlags(searchFlags);
  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Ex->MarkerLine('#')));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('$')));

  int nr_replacements = 0;
  int result = wxID_YES;
  
  while (m_STC->SearchInTarget(pattern) != -1 && result != wxID_CANCEL)
  {
    const wxString replacement(BuildReplacement(repl));
    
    if (options.Contains("c"))
    {
      result = Confirm(pattern, replacement, indicator);
    }
        
    if (result == wxID_YES)
    {
      (searchFlags & wxSTC_FIND_REGEXP) ?
        m_STC->ReplaceTargetRE(replacement):
        m_STC->ReplaceTarget(replacement);
        
      nr_replacements++;
    }
    
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

    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('$')));
  
    if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
    {
      break;
    }
  }

  m_STC->EndUndoAction();
  
  if (m_Begin == "'<" && m_End == "'>")
  {
    m_STC->SetSelection(
      m_STC->PositionFromLine(m_Ex->MarkerLine('#')),
      m_STC->PositionFromLine(m_Ex->MarkerLine('$') + corrected));
  }

  m_Ex->MarkerDelete('#');
  m_Ex->MarkerDelete('$');
  
  m_Ex->GetFrame()->ShowExMessage(wxString::Format(
    _("Replaced: %d occurrences of: %s"), nr_replacements, pattern.c_str()));

  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  return true;
}

bool wxExAddressRange::Write(const wxString& text) const
{
  if (!SetSelection())
  {
    return false;
  }
  
  const wxString filename(wxString(text.Contains(">>") ? text.AfterLast('>'): text).Trim(false));
  const wxFile::OpenMode mode(text.Contains(">>") ? wxFile::write_append: wxFile::write);

  wxFile file(filename, mode);

  return 
    file.IsOpened() && 
    file.Write(m_Ex->GetSelectedText());
}

bool wxExAddressRange::Yank(const char name) const
{
  if (!SetSelection())
  {
    return false;
  }
  
  m_Ex->GetMacros().SetRegister(name, m_Ex->GetSelectedText());

  return true;
}

#endif // wxUSE_GUI
