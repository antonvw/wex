////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.cpp
// Purpose:   Implementation of class wxExAddressRange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/addressrange.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/process.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>
#include <wx/extension/vi-macros.h>

#if wxUSE_GUI

class GlobalEnv
{
public:
  GlobalEnv(wxExEx* ex, 
    const wxExIndicator& indicator, const std::string& commands)
  : m_Ex(ex)
  , m_FindIndicator(indicator) {
    m_Ex->GetSTC()->SetSearchFlags(m_Ex->GetSearchFlags());
    m_Ex->GetSTC()->BeginUndoAction();
    
    wxExTokenizer tkz(commands, "|");
    
    while (tkz.HasMoreTokens())
    {
      const std::string cmd(tkz.GetNextToken());
      
      // Prevent recursive global.
      if (cmd[0] != 'g' && cmd[0] != 'v')
      {
        if (cmd[0] == 'd' || cmd[0] == 'm')
        {
          m_Changes++;
        }
        
        m_Commands.emplace_back(cmd);
      }
    }}
  
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
          m_Ex->GetFrame()->ShowExMessage(cmd + " failed");
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
  int m_Changes = 0;
  wxExEx* m_Ex;
};

std::string wxExAddressRange::m_Pattern;
std::string wxExAddressRange::m_Replacement;
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

wxExAddressRange::wxExAddressRange(wxExEx* ex, const std::string& range)
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
  else if (range.find(",") != std::string::npos)
  {
    Set(range.substr(0, range.find(",")), range.substr(range.find(",") + 1));
  }
  else
  {
    Set(range, range);
  }
}

const std::string wxExAddressRange::BuildReplacement(const std::string& text) const
{
  if (text.find("&") == std::string::npos && 
      text.find("\0") == std::string::npos)
  {
    return text;
  }

  std::string target(m_STC->GetTextRange(
    m_STC->GetTargetStart(), m_STC->GetTargetEnd()));
    
  std::string replacement;
  bool backslash = false;

  for (const auto c : text)
  {
    switch (c)
    {
      case '&': 
        if (!backslash) 
          replacement += target; 
        else
          replacement += c;
        backslash = false; 
        break;
        
      case '0': 
        if (backslash) 
          replacement += target; 
        else
          replacement += c;
        backslash = false; 
        break;
        
      case 'L': 
        if (backslash) 
          std::transform(target.begin(), target.end(), target.begin(), ::tolower);
        else
          replacement += c;
        backslash = false; 
        break;
        
      case 'U': 
        if (backslash) 
          std::transform(target.begin(), target.end(), target.begin(), ::toupper);
        else
          replacement += c;
        backslash = false; 
        break;
        
      case '\\': 
        if (backslash) 
          replacement += c;
        backslash = !backslash; 
        break;
        
      default:
        replacement += c;
        backslash = false; 
    }
  }

  return replacement;
}
  
bool wxExAddressRange::Change(const std::string& text) const
{
  if (!Delete())
  {
    return false;
  }
  
  m_Ex->GetSTC()->AddText(text);
  
  return true;
}
  
void wxExAddressRange::Cleanup()
{
  delete m_Process;
}
  
int wxExAddressRange::Confirm(
  const std::string& pattern, const std::string& replacement)
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
    m_FindIndicator, m_STC->GetTargetStart(), m_STC->GetTargetEnd());
  
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
      wxString::Format(_("%d lines copied"), lines - 1).ToStdString());
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

bool wxExAddressRange::Escape(const std::string& command)
{
  if (m_Begin.m_Address.empty() && m_End.m_Address.empty())
  {
    std::string expanded(command);

    if (
      !wxExMarkerAndRegisterExpansion(m_Ex, expanded) ||
      !wxExShellExpansion(expanded)) return false;

    if (m_Process == nullptr)
    {
      m_Process = new wxExProcess();
    }
    else
    {
      m_Process->Kill();
    }
    
    return m_Process->Execute(expanded, false, m_STC->GetFileName().GetPath());
  }
  
  if (!IsOk())
  {
    return false;
  }
  
  const std::string filename("__TMPFILE__");
  
  if (m_STC->GetReadOnly() || m_STC->HexMode() || !Write(filename))
  {
    return false;
  }

  wxExProcess process;
  
  const bool ok = process.Execute(command + " " + filename, true);
  
  if (remove(filename.c_str()) != 0)
  {
    wxLogStatus("Could not remove file");
  }
  
  if (ok)
  {
    if (!process.GetStdOut().empty())
    {      
      m_STC->BeginUndoAction();

      if (Delete(false))
      {
        m_STC->AddText(process.GetStdOut());
        m_STC->AddText(m_STC->GetEOL());
      }
      
      m_STC->EndUndoAction();
      
      return true;
    }
    else if (!process.GetStdErr().empty())
    {
      m_Ex->GetFrame()->ShowExMessage(process.GetStdErr());
    }
  }
  
  return false;
}

bool wxExAddressRange::Global(const std::string& text, bool inverse) const
{
  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  wxExTokenizer next(text, "/", false);

  if (next.CountTokens() <= 1)
  {
    return true;
  }

  next.GetNextToken(); // skip empty token

  const std::string pattern = next.GetNextToken();
  std::string rest;
  
  if (next.HasMoreTokens())
  {
    const std::string token(next.GetNextToken());
    
    if (!token.empty())
    {
      const char command = token[0];
      std::string arg(token.size() > 1 ? token.substr(1): std::string());
      
      if (next.HasMoreTokens())
      {
        std::string subpattern = next.GetNextToken();
        
        if (subpattern.empty())
        {
          subpattern = pattern;
        }

        arg += "/" + subpattern + "/" + next.GetString();
      }
      
      rest = std::string(1, command) + arg;
    }
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
  
  const GlobalEnv g(m_Ex, m_FindIndicator, rest);
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
      m_Ex->GetFrame()->ShowExMessage(
        "possible infinite loop at " + std::to_string(match));
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
      m_Ex->GetFrame()->ShowExMessage(
        wxString::Format(_("Executed: %d commands"), hits).ToStdString());
    else
      m_Ex->GetFrame()->ShowExMessage(
        wxString::Format(_("Found: %d matches"), hits).ToStdString());
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
      wxString::Format(_("%d lines moved"), lines - 1).ToStdString());
  }

  return true;
}

bool wxExAddressRange::Parse(
  const std::string& command_org, 
  std::string& pattern, std::string& replacement, std::string& options) const
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use string tokenizer with / as separator.
  bool escaped = false;
  
  std::string command(command_org);
  
  if (command.find("\\\\/") == std::string::npos && 
      command.find("\\/") != std::string::npos)
  {
    if (command.find(char(1)) == std::string::npos)
    {
      wxExReplaceAll(command, "\\/", "\x01");
      escaped = true;
    }
    else
    {
      wxLogStatus("Internal char exists");
      return false;
    }
  }

  std::vector<std::string> v;

  if (wxExMatch("/(.*)/(.*)/([cgi]*)", command, v) == 3 ||
      wxExMatch("/(.*)/(.*)", command, v) == 2 ||
      wxExMatch("/(.*)", command, v) == 1)
  {
    pattern = v[0];
    if (v.size() >= 2) replacement = v[1];
    if (v.size() >= 3) options = v[2];
    
    // Restore a / for all occurrences of the special char.
    if (escaped)
    {  
      std::replace(pattern.begin(), pattern.end(), '\x01', '/');
      std::replace(replacement.begin(), replacement.end(), '\x01', '/');
    }

    return true;
  }
  
  return false;
}
    
bool wxExAddressRange::Print(const std::string& flags) const
{
  if (!IsOk() || !m_Begin.Flags(flags))
  {
    return false;
  }
  
  std::string text;
  
  for (int i = m_Begin.GetLine() - 1; i < m_End.GetLine(); i++)
  {
    char buffer[8];
    sprintf(buffer, "%6d ", i + 1);
    
    text += (flags.find("#") != std::string::npos ? buffer: std::string()) + 
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

bool wxExAddressRange::Sort(const std::string& parameters) const
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
         ( parameters.find("u") != 0 && 
           parameters.find("r") != 0 && 
          !isdigit(parameters[0])))
    {
      return false;
    }
    
    sort_type |= (parameters.find("r") != std::string::npos ? STRING_SORT_DESCENDING: 0);
    sort_type |= (parameters.find("u")!= std::string::npos  ? STRING_SORT_UNIQUE: 0);
    
    if (isdigit(parameters[0]))
    {
      try
      {
        pos = (std::stoi(parameters) > 0 ? std::stoi(parameters) - 1: 0);
        
        if (parameters.find(",") != std::string::npos)
        {
          len = std::stoi(parameters.substr(parameters.find(',') + 1)) - pos + 1;
        }
      }
      catch (...)
      {
      }
    }
  }

  return wxExSortSelection(m_STC, sort_type, pos, len);
}
  
bool wxExAddressRange::Substitute(const std::string& text, const char cmd)
{
  if (m_STC->GetReadOnly() || !IsOk())
  {
    return false;
  }
  
  std::string pattern;
  std::string repl;
  std::string options;
  
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
    
  if (pattern.empty())
  {
    wxLogStatus("Pattern is empty");
    return false;
  }

  int searchFlags = m_Ex->GetSearchFlags();
  if (options.find("i") != std::string::npos) searchFlags &= ~wxSTC_FIND_MATCHCASE;
    
  if ((searchFlags & wxSTC_FIND_REGEXP) && 
    pattern.size() == 2 && pattern.back() == '*' && repl.empty())
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

  m_Pattern = pattern;
  m_Replacement = repl; 
  
  m_STC->SetSearchFlags(searchFlags);
  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Ex->MarkerLine('#')));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('$')));

  int nr_replacements = 0;
  int result = wxID_YES;
  const bool build = (repl.find_first_of("&0LU\\") != std::string::npos);
  const bool confirm = (options.find("c") != std::string::npos);
  const bool global = (options.find("g") != std::string::npos);
  std::string replacement(repl);

  while (m_STC->SearchInTarget(pattern) != -1 && result != wxID_CANCEL)
  {
    if (build)
    {
      replacement = BuildReplacement(repl);
    }
    
    if (confirm)
    {
      result = Confirm(pattern, replacement);
    }
        
    if (result == wxID_YES)
    {
      if (m_STC->HexMode())
      {  
        m_STC->GetHexMode().ReplaceTarget(replacement, false);
      }
      else
      {
        (searchFlags & wxSTC_FIND_REGEXP) ?
           m_STC->ReplaceTargetRE(replacement):
           m_STC->ReplaceTarget(replacement);
      }
        
      nr_replacements++;
    }
    
    m_STC->SetTargetStart(global ? 
      m_STC->GetTargetEnd():
      m_STC->GetLineEndPosition(m_STC->LineFromPosition(m_STC->GetTargetEnd())));
    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->MarkerLine('$')));
  
    if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
    {
      break;
    }
  }
  
  if (m_STC->HexMode())
  {
    m_STC->GetHexMode().SetText(m_STC->GetHexMode().GetBuffer());
  }

  m_STC->EndUndoAction();
  
  if (m_Begin.m_Address == "'<" && m_End.m_Address == "'>")
  {
    m_STC->SetSelection(
      m_STC->PositionFromLine(m_Ex->MarkerLine('#')),
      m_STC->PositionFromLine(m_Ex->MarkerLine('$') + corrected));
  }

  m_Ex->MarkerDelete('#');
  m_Ex->MarkerDelete('$');
  
  m_Ex->GetFrame()->ShowExMessage(wxString::Format(
    _("Replaced: %d occurrences of: %s"), nr_replacements, pattern.c_str()).ToStdString());

  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  return true;
}

bool wxExAddressRange::Write(const std::string& text) const
{
  if (!SetSelection())
  {
    return false;
  }
  
  std::string filename(wxExSkipWhiteSpace(text.find(">>") != std::string::npos ? 
    wxExAfter(text, '>', false): text, SKIP_LEFT));

#ifdef __UNIX__
  if (filename.find("~") != std::string::npos)
  {
    filename.replace(filename.find("~"), 1, wxGetHomeDir().ToStdString());
  }
#endif

  return wxExFile(filename, text.find(">>") != std::string::npos ? 
    wxFile::write_append: wxFile::write).Write(m_Ex->GetSelectedText());
}

bool wxExAddressRange::Yank(const char name) const
{
  return 
    SetSelection() &&
    m_Ex->GetMacros().SetRegister(name, m_Ex->GetSelectedText());
}
#endif // wxUSE_GUI
