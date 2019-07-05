////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.cpp
// Purpose:   Implementation of class wex::addressrange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/addressrange.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/managedframe.h>
#include <wex/process.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/util.h>
#include <wex/vi-macros.h>

namespace wex
{
  class global_env
  {
  public:
    global_env(ex* ex, 
      const indicator& indicator, const std::string& commands)
    : m_Ex(ex)
    , m_FindIndicator(indicator) {
      m_Ex->get_stc()->set_search_flags(m_Ex->search_flags());
      m_Ex->get_stc()->BeginUndoAction();
      
      for (tokenizer tkz(commands, "|"); tkz.has_more_tokens(); )
      {
        // Prevent recursive global.
        if (const auto cmd(tkz.get_next_token()); cmd[0] != 'g' && cmd[0] != 'v')
        {
          if (cmd[0] == 'd' || cmd[0] == 'm')
          {
            m_Changes++;
          }
          
          m_Commands.emplace_back(cmd);
        }
      }}
    
   ~global_env()
    {
      m_Ex->get_stc()->EndUndoAction();
      m_Ex->marker_delete('%');
    }
    
    auto changes() const {return m_Changes;};
    
    bool commands() const {return !m_Commands.empty();};
    
    bool for_each(int line) const
    {
      if (!commands())
      {
        m_Ex->get_stc()->set_indicator(m_FindIndicator, 
          m_Ex->get_stc()->GetTargetStart(), m_Ex->get_stc()->GetTargetEnd());
      }
      else
      {
        for (const auto& it : m_Commands)
        {
          if (!m_Ex->command(":" + std::to_string(line + 1) + it))
          {
            m_Ex->frame()->show_ex_message(m_Ex->get_command().command() + " failed");
            return false;
          }
        }
      }
      
      return true;
    }

    bool for_each(int start, int& end, int& hits) const
    {
      if (start < end)
      {
        for (int i = start;  i < end && i < m_Ex->get_stc()->GetLineCount() - 1; )
        {
          if (commands())
          {
            if (!for_each(i)) return false;
          }
          else
          {
            m_Ex->get_stc()->set_indicator(
              m_FindIndicator, 
              m_Ex->get_stc()->PositionFromLine(i), 
              m_Ex->get_stc()->GetLineEndPosition(i));
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
    const indicator m_FindIndicator;
    std::vector<std::string> m_Commands;
    int m_Changes {0};
    ex* m_Ex;
  };
};

wex::addressrange::addressrange(wex::ex* ex, int lines)
  : m_Begin(ex)
  , m_End(ex)
  , m_Ex(ex)
  , m_STC(ex->get_stc())
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

wex::addressrange::addressrange(wex::ex* ex, const std::string& range)
  : m_Begin(ex)
  , m_End(ex)
  , m_Ex(ex)
  , m_STC(ex->get_stc())
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

const std::string wex::addressrange::BuildReplacement(const std::string& text) const
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
        {
          std::transform(target.begin(), target.end(), target.begin(), ::tolower);
          m_STC->Replace(m_STC->GetTargetStart(), m_STC->GetTargetEnd(), target);
        }
        else
          replacement += c;
        backslash = false; 
        break;
        
      case 'U': 
        if (backslash) 
        {
          std::transform(target.begin(), target.end(), target.begin(), ::toupper);
          m_STC->Replace(m_STC->GetTargetStart(), m_STC->GetTargetEnd(), target);
        }
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
  
bool wex::addressrange::change(const std::string& text) const
{
  if (!erase())
  {
    return false;
  }
  
  m_Ex->add_text(text);
  
  return true;
}
  
int wex::addressrange::Confirm(
  const std::string& pattern, const std::string& replacement)
{
  wxMessageDialog msgDialog(m_STC, 
    _("Replace") + " " + pattern + " " + _("with") + " " + replacement, 
    _("Replace"), 
    wxCANCEL | wxYES_NO);
    
  const auto line = m_STC->LineFromPosition(m_STC->GetTargetStart());
  
  msgDialog.SetExtendedMessage("Line " + std::to_string(line + 1) + ": " + 
    m_STC->GetLineText(line));
    
  m_STC->GotoLine(line);
  m_STC->EnsureVisible(line);
  m_STC->set_indicator(
    m_FindIndicator, m_STC->GetTargetStart(), m_STC->GetTargetEnd());
  
  return msgDialog.ShowModal();
}

bool wex::addressrange::copy(const wex::address& destination) const
{
  const auto dest_line = destination.get_line();

  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !is_ok() ||
     dest_line == 0 || 
    (dest_line >= m_Begin.get_line() && dest_line <= m_End.get_line()))
  {
    return false;
  }

  m_STC->BeginUndoAction();

  if (yank())
  {
    m_STC->GotoLine(dest_line - 1);
    m_Ex->add_text(m_Ex->register_text());
  }

  m_STC->EndUndoAction();
  
  const auto lines = get_number_of_lines(m_Ex->register_text());

  if (lines >= 2)
  {
    m_Ex->frame()->show_ex_message(
      wxString::Format(_("%d lines copied"), lines - 1).ToStdString());
  }

  return true;
}
  
bool wex::addressrange::erase(bool show_message) const
{
  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !SetSelection())
  {
    return false;
  }

  m_Ex->cut(show_message);
  
  m_Begin.marker_delete();
  m_End.marker_delete();

  return true;
}

bool wex::addressrange::escape(const std::string& command)
{
  if (m_Begin.m_Address.empty() && m_End.m_Address.empty())
  {
    auto expanded(command);

    if (
      !marker_and_register_expansion(m_Ex, expanded) ||
      !shell_expansion(expanded)) return false;

    // TODO: here is a leak, otherwise test-ex fails
    m_Process = new wex::process();

    return m_Process->execute(expanded, 
      process::EXEC_NO_WAIT, m_STC->get_filename().get_path());
  }
  
  if (!is_ok())
  {
    return false;
  }
  
  const std::string tmp_filename(path(
    std::filesystem::temp_directory_path().string(),
    std::to_string(std::time(nullptr))).data().string());
  
  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !write(tmp_filename))
  {
    return false;
  }

  wex::process process;
  
  const bool ok = process.execute(
    command + " " + tmp_filename, process::EXEC_WAIT);
  
  if (remove(tmp_filename.c_str()) != 0)
  {
    log::status("Could not remove file");
  }
  
  if (ok)
  {
    if (!process.get_stdout().empty())
    {      
      m_STC->BeginUndoAction();

      if (erase(false))
      {
        m_STC->AddText(process.get_stdout());
      }
      
      m_STC->EndUndoAction();
      
      return true;
    }
    else if (!process.get_stderr().empty())
    {
      m_Ex->frame()->show_ex_message(process.get_stderr());
    }
  }
  
  return false;
}

bool wex::addressrange::global(const std::string& text, bool inverse) const
{
  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  tokenizer next(text, "/", false);

  if (next.count_tokens() <= 1)
  {
    return false;
  }

  next.get_next_token(); // skip empty token

  const auto pattern = next.get_next_token();
  std::string rest;
  
  if (next.has_more_tokens())
  {
    if (const auto token(next.get_next_token()); !token.empty())
    {
      const auto command = token[0];
      auto arg(token.size() > 1 ? token.substr(1): std::string());
      
      if (next.has_more_tokens())
      {
        auto subpattern = next.get_next_token();
        
        if (subpattern.empty())
        {
          subpattern = pattern;
        }

        arg += "/" + subpattern + "/" + next.get_string();
      }
      
      rest = std::string(1, command) + arg;
    }
  }

  if (pattern.empty())
  {
    if (!rest.empty())
    {
      log::status("Pattern is empty");
      return false;
    }
    
    return true;  
  }
  
  const global_env g(m_Ex, m_FindIndicator, rest);
  m_Ex->marker_add('%', m_End.get_line() - 1);
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Begin.get_line() - 1));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->marker_line('%')));
  
  const bool infinite = (g.changes() > 0 && rest != "$" && rest != "1");
  int hits = 0;
  int start = 0;
  
  while (m_STC->SearchInTarget(pattern) != -1)
  {
    auto match = m_STC->LineFromPosition(m_STC->GetTargetStart());
    
    if (!inverse)
    {
      if (!g.for_each(match)) return false;
      hits++;
    }
    else
    {
      if (!g.for_each(start, match, hits)) return false;
      start = match + 1;
    }
        
    if (hits > 50 && infinite)
    {
      m_Ex->frame()->show_ex_message(
        "possible infinite loop at " + std::to_string(match));
      return false;
    }
    
    m_STC->SetTargetStart(g.changes() > 0 ? m_STC->PositionFromLine(match): m_STC->GetTargetEnd());
    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->marker_line('%')));
  
    if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
    {
      break;
    }
  }
  
  if (inverse)
  {
    if (auto match = m_STC->GetLineCount(); !g.for_each(start, match, hits)) return false;
  }
  
  if (hits > 0)
  {
    if (g.commands())
      m_Ex->frame()->show_ex_message(
        wxString::Format(_("Executed: %d commands"), hits).ToStdString());
    else
      m_Ex->frame()->show_ex_message(
        wxString::Format(_("Found: %d matches"), hits).ToStdString());
  }
  
  return true;
}

bool wex::addressrange::Indent(bool forward) const
{
  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !is_ok() || !SetSelection())
  {
    return false;
  }
  
  m_STC->BeginUndoAction();
  m_STC->SendMsg(forward ? wxSTC_CMD_TAB: wxSTC_CMD_BACKTAB);
  m_STC->EndUndoAction();
  
  return true;
}

bool wex::addressrange::is_ok() const
{
  return 
    m_Begin.get_line() > 0 && m_End.get_line() > 0 &&
    m_Begin.get_line() <= m_End.get_line();
}

bool wex::addressrange::join() const
{
  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !is_ok())
  {
    return false;
  }
  
  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Begin.get_line() - 1));
  m_STC->SetTargetEnd(m_STC->PositionFromLine(m_End.get_line()));
  m_STC->LinesJoin();
  m_STC->EndUndoAction();
  
  return true;
}
  
bool wex::addressrange::move(const address& destination) const
{
  const auto dest_line = destination.get_line();

  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !is_ok() ||
     dest_line == 0 || 
    (dest_line >= m_Begin.get_line() && dest_line <= m_End.get_line()))
  {
    return false;
  }

  m_STC->BeginUndoAction();

  if (erase(false))
  {
    m_STC->GotoLine(dest_line - 1);
    m_Ex->add_text(m_Ex->register_text());
  }

  m_STC->EndUndoAction();
  
  const auto lines = get_number_of_lines(m_Ex->register_text());

  if (lines >= 2)
  {
    m_Ex->frame()->show_ex_message(
      wxString::Format(_("%d lines moved"), lines - 1).ToStdString());
  }

  return true;
}

void wex::addressrange::on_exit()
{
  delete m_Process;
}
  
bool wex::addressrange::Parse(
  const std::string& command_org, 
  std::string& pattern, std::string& replacement, std::string& options) const
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use string tokenizer with / as separator.
  bool escaped = false;
  
  auto command(command_org);
  
  if (command.find("\\\\/") == std::string::npos && 
      command.find("\\/") != std::string::npos)
  {
    if (command.find(char(1)) == std::string::npos)
    {
      replace_all(command, "\\/", "\x01");
      escaped = true;
    }
    else
    {
      log::status("Internal char exists");
      return false;
    }
  }

  if (std::vector<std::string> v;
    match("/(.*)/(.*)/([cgi]*)", command, v) == 3 ||
    match("/(.*)/(.*)", command, v) == 2 ||
    match("/(.*)", command, v) == 1)
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
    
bool wex::addressrange::print(const std::string& flags) const
{
  if (!is_ok() || !m_Begin.flags_supported(flags))
  {
    return false;
  }
  
  std::string text;
  
  for (auto i = m_Begin.get_line() - 1; i < m_End.get_line(); i++)
  {
    char buffer[8];
    sprintf(buffer, "%6d ", i + 1);
    
    text += (flags.find("#") != std::string::npos ? buffer: std::string()) + 
      m_STC->GetLine(i);
  }
    
  m_Ex->frame()->print_ex(m_Ex, text);
  
  return true;
}
  
void wex::addressrange::Set(address& begin, address& end, int lines)
{
  begin.SetLine(m_STC->LineFromPosition(m_STC->GetCurrentPos()) + 1);
  end.SetLine(begin.get_line() + lines - 1);
}

bool wex::addressrange::SetSelection() const
{
  if (!m_STC->GetSelectedText().empty())
  {
    return true;
  }
  else if (!is_ok())
  {
    return false;
  }
  
  m_STC->SetSelection(
    m_STC->PositionFromLine(m_Begin.get_line() - 1),
    m_STC->PositionFromLine(m_End.get_line()));

  return true;
}

bool wex::addressrange::sort(const std::string& parameters) const
{
  if (m_STC->GetReadOnly() || m_STC->is_hexmode() || !SetSelection())
  {
    return false;
  }
  
  string_sort_t sort_t = 0;
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
    
    if (parameters.find("r") != std::string::npos) sort_t.set(STRING_SORT_DESCENDING);
    if (parameters.find("u") != std::string::npos) sort_t.set(STRING_SORT_UNIQUE);
    
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

  return sort_selection(m_STC, sort_t, pos, len);
}
  
bool wex::addressrange::substitute(const std::string& text, const char cmd)
{
  if (m_STC->GetReadOnly() || !is_ok())
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
    log::status("Pattern is empty");
    return false;
  }

  auto searchFlags = m_Ex->search_flags();
  if (options.find("i") != std::string::npos) searchFlags &= ~wxSTC_FIND_MATCHCASE;
    
  if ((searchFlags & wxSTC_FIND_REGEXP) && 
    pattern.size() == 2 && pattern.back() == '*' && repl.empty())
  {
    log::status("Replacement leads to infinite loop");
    return false;
  }
       
  if (!m_Ex->marker_add('#', m_Begin.get_line() - 1))
  {
    return false;
  }

  int corrected = 0;
  auto end_line = m_End.get_line() - 1;
  
  if (!m_STC->GetSelectedText().empty())
  {
    if (m_STC->GetLineSelEndPosition(end_line) == m_STC->PositionFromLine(end_line))
    {
      end_line--;
      corrected = 1;
    }
  }
  
  if (!m_Ex->marker_add('$', end_line))
  {
    return false;
  }

  m_Pattern = pattern;
  m_Replacement = repl; 
  
  m_STC->set_search_flags(searchFlags);
  m_STC->BeginUndoAction();
  m_STC->SetTargetStart(m_STC->PositionFromLine(m_Ex->marker_line('#')));
  m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->marker_line('$')));

  int nr_replacements = 0;
  int result = wxID_YES;
  const bool build = (repl.find_first_of("&0LU\\") != std::string::npos);
  const bool confirm = (options.find("c") != std::string::npos);
  const bool global = (options.find("g") != std::string::npos);
  auto replacement(repl);

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
      if (m_STC->is_hexmode())
      {  
        m_STC->get_hexmode().replace_target(replacement, false);
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
    m_STC->SetTargetEnd(m_STC->GetLineEndPosition(m_Ex->marker_line('$')));
  
    if (m_STC->GetTargetStart() >= m_STC->GetTargetEnd())
    {
      break;
    }
  }
  
  if (m_STC->is_hexmode())
  {
    m_STC->get_hexmode().set_text(m_STC->get_hexmode().buffer());
  }

  m_STC->EndUndoAction();
  
  if (m_Begin.m_Address == "'<" && m_End.m_Address == "'>")
  {
    m_STC->SetSelection(
      m_STC->PositionFromLine(m_Ex->marker_line('#')),
      m_STC->PositionFromLine(m_Ex->marker_line('$') + corrected));
  }

  m_Ex->marker_delete('#');
  m_Ex->marker_delete('$');
  
  m_Ex->frame()->show_ex_message(wxString::Format(
    _("Replaced: %d occurrences of: %s"), nr_replacements, pattern.c_str()).ToStdString());

  m_STC->IndicatorClearRange(0, m_STC->GetTextLength() - 1);
  
  return true;
}

bool wex::addressrange::write(const std::string& text) const
{
  if (!SetSelection())
  {
    return false;
  }
  
  auto filename(skip_white_space(text.find(">>") != std::string::npos ? 
    wex::after(text, '>', false): text, 
    skip_t().set(SKIP_LEFT)));

#ifdef __UNIX__
  if (filename.find("~") != std::string::npos)
  {
    filename.replace(filename.find("~"), 1, wxGetHomeDir().ToStdString());
  }
#endif

  return wex::file(filename, text.find(">>") != std::string::npos ? 
    std::ios_base::app: std::ios::out).write(m_STC->get_selected_text());
}

bool wex::addressrange::yank(const char name) const
{
  return SetSelection() && m_Ex->yank(name);
}
