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
    : m_ex(ex)
    , m_find_indicator(indicator) {
      m_ex->get_stc()->set_search_flags(m_ex->search_flags());
      m_ex->get_stc()->BeginUndoAction();
      
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
      m_ex->get_stc()->EndUndoAction();
      m_ex->marker_delete('%');
    }
    
    auto changes() const {return m_Changes;};
    
    bool commands() const {return !m_Commands.empty();};
    
    bool for_each(int line) const
    {
      if (!commands())
      {
        m_ex->get_stc()->set_indicator(m_find_indicator, 
          m_ex->get_stc()->GetTargetStart(), m_ex->get_stc()->GetTargetEnd());
      }
      else
      {
        for (const auto& it : m_Commands)
        {
          if (!m_ex->command(":" + std::to_string(line + 1) + it))
          {
            m_ex->frame()->show_ex_message(m_ex->get_command().command() + " failed");
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
        for (int i = start;  i < end && i < m_ex->get_stc()->GetLineCount() - 1; )
        {
          if (commands())
          {
            if (!for_each(i)) return false;
          }
          else
          {
            m_ex->get_stc()->set_indicator(
              m_find_indicator, 
              m_ex->get_stc()->PositionFromLine(i), 
              m_ex->get_stc()->GetLineEndPosition(i));
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
    const indicator m_find_indicator;
    std::vector<std::string> m_Commands;
    int m_Changes {0};
    ex* m_ex;
  };
};

wex::addressrange::addressrange(wex::ex* ex, int lines)
  : m_begin(ex)
  , m_end(ex)
  , m_ex(ex)
  , m_stc(ex->get_stc())
{
  if (lines > 0) 
  {
    set(m_begin, m_end, lines);
  }
  else if (lines < 0)
  {
    set(m_end, m_begin, lines);
  }
}

wex::addressrange::addressrange(wex::ex* ex, const std::string& range)
  : m_begin(ex)
  , m_end(ex)
  , m_ex(ex)
  , m_stc(ex->get_stc())
{
  if (range == "%")
  {
    set("1", "$");
  }
  else if (range == "*")
  {
    set(
      m_stc->GetFirstVisibleLine() + 1, 
      m_stc->GetFirstVisibleLine() + m_stc->LinesOnScreen() + 1);
  }
  else if (range.find(",") != std::string::npos)
  {
    set(range.substr(0, range.find(",")), range.substr(range.find(",") + 1));
  }
  else
  {
    set(range, range);
  }
}

const std::string wex::addressrange::build_replacement(
  const std::string& text) const
{
  if (text.find("&") == std::string::npos && 
      text.find("\0") == std::string::npos)
  {
    return text;
  }

  std::string target(m_stc->GetTextRange(
    m_stc->GetTargetStart(), m_stc->GetTargetEnd()));
    
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
          m_stc->Replace(m_stc->GetTargetStart(), m_stc->GetTargetEnd(), target);
        }
        else
          replacement += c;
        backslash = false; 
        break;
        
      case 'U': 
        if (backslash) 
        {
          std::transform(target.begin(), target.end(), target.begin(), ::toupper);
          m_stc->Replace(m_stc->GetTargetStart(), m_stc->GetTargetEnd(), target);
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
  
  m_ex->add_text(text);
  
  return true;
}
  
int wex::addressrange::confirm(
  const std::string& pattern, const std::string& replacement)
{
  wxMessageDialog msgDialog(m_stc, 
    _("Replace") + " " + pattern + " " + _("with") + " " + replacement, 
    _("Replace"), 
    wxCANCEL | wxYES_NO);
    
  const auto line = m_stc->LineFromPosition(m_stc->GetTargetStart());
  
  msgDialog.SetExtendedMessage("Line " + std::to_string(line + 1) + ": " + 
    m_stc->GetLineText(line));
    
  m_stc->GotoLine(line);
  m_stc->EnsureVisible(line);
  m_stc->set_indicator(
    m_find_indicator, m_stc->GetTargetStart(), m_stc->GetTargetEnd());
  
  return msgDialog.ShowModal();
}

bool wex::addressrange::copy(const wex::address& destination) const
{
  const auto dest_line = destination.get_line();

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() ||
     dest_line == 0 || 
    (dest_line >= m_begin.get_line() && dest_line <= m_end.get_line()))
  {
    return false;
  }

  m_stc->BeginUndoAction();

  if (yank())
  {
    m_stc->GotoLine(dest_line - 1);
    m_ex->add_text(m_ex->register_text());
  }

  m_stc->EndUndoAction();
  
  const auto lines = get_number_of_lines(m_ex->register_text());

  if (lines >= 2)
  {
    m_ex->frame()->show_ex_message(
      wxString::Format(_("%d lines copied"), lines - 1).ToStdString());
  }

  return true;
}
  
bool wex::addressrange::erase(bool show_message) const
{
  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !set_selection())
  {
    return false;
  }

  m_ex->cut(show_message);
  
  m_begin.marker_delete();
  m_end.marker_delete();

  return true;
}

bool wex::addressrange::escape(const std::string& command)
{
  if (m_begin.m_address.empty() && m_end.m_address.empty())
  {
    auto expanded(command);

    if (
      !marker_and_register_expansion(m_ex, expanded) ||
      !shell_expansion(expanded)) return false;

    // TODO: here is a leak, otherwise test-ex fails
    m_process = new wex::process();

    return m_process->execute(expanded, 
      process::EXEC_NO_WAIT, m_stc->get_filename().get_path());
  }
  
  if (!is_ok())
  {
    return false;
  }
  
  const std::string tmp_filename(path(
    std::filesystem::temp_directory_path().string(),
    std::to_string(std::time(nullptr))).data().string());
  
  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !write(tmp_filename))
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
      m_stc->BeginUndoAction();

      if (erase(false))
      {
        m_stc->AddText(process.get_stdout());
      }
      
      m_stc->EndUndoAction();
      
      return true;
    }
    else if (!process.get_stderr().empty())
    {
      m_ex->frame()->show_ex_message(process.get_stderr());
    }
  }
  
  return false;
}

bool wex::addressrange::global(const std::string& text, bool inverse) const
{
  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
  
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
  
  const global_env g(m_ex, m_find_indicator, rest);
  m_ex->marker_add('%', m_end.get_line() - 1);
  m_stc->SetTargetStart(m_stc->PositionFromLine(m_begin.get_line() - 1));
  m_stc->SetTargetEnd(m_stc->GetLineEndPosition(m_ex->marker_line('%')));
  
  const bool infinite = (g.changes() > 0 && rest != "$" && rest != "1");
  int hits = 0;
  int start = 0;
  
  while (m_stc->SearchInTarget(pattern) != -1)
  {
    auto match = m_stc->LineFromPosition(m_stc->GetTargetStart());
    
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
      m_ex->frame()->show_ex_message(
        "possible infinite loop at " + std::to_string(match));
      return false;
    }
    
    m_stc->SetTargetStart(g.changes() > 0 ? m_stc->PositionFromLine(match): m_stc->GetTargetEnd());
    m_stc->SetTargetEnd(m_stc->GetLineEndPosition(m_ex->marker_line('%')));
  
    if (m_stc->GetTargetStart() >= m_stc->GetTargetEnd())
    {
      break;
    }
  }
  
  if (inverse)
  {
    if (auto match = m_stc->GetLineCount(); !g.for_each(start, match, hits)) return false;
  }
  
  if (hits > 0)
  {
    if (g.commands())
      m_ex->frame()->show_ex_message(
        wxString::Format(_("Executed: %d commands"), hits).ToStdString());
    else
      m_ex->frame()->show_ex_message(
        wxString::Format(_("Found: %d matches"), hits).ToStdString());
  }
  
  return true;
}

bool wex::addressrange::indent(bool forward) const
{
  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() || !set_selection())
  {
    return false;
  }
  
  m_stc->BeginUndoAction();
  m_stc->SendMsg(forward ? wxSTC_CMD_TAB: wxSTC_CMD_BACKTAB);
  m_stc->EndUndoAction();
  
  return true;
}

bool wex::addressrange::is_ok() const
{
  return 
    m_begin.get_line() > 0 && m_end.get_line() > 0 &&
    m_begin.get_line() <= m_end.get_line();
}

bool wex::addressrange::join() const
{
  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok())
  {
    return false;
  }
  
  m_stc->BeginUndoAction();
  m_stc->SetTargetStart(m_stc->PositionFromLine(m_begin.get_line() - 1));
  m_stc->SetTargetEnd(m_stc->PositionFromLine(m_end.get_line()));
  m_stc->LinesJoin();
  m_stc->EndUndoAction();
  
  return true;
}
  
bool wex::addressrange::move(const address& destination) const
{
  const auto dest_line = destination.get_line();

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() ||
     dest_line == 0 || 
    (dest_line >= m_begin.get_line() && dest_line <= m_end.get_line()))
  {
    return false;
  }

  m_stc->BeginUndoAction();

  if (erase(false))
  {
    m_stc->GotoLine(dest_line - 1);
    m_ex->add_text(m_ex->register_text());
  }

  m_stc->EndUndoAction();
  
  const auto lines = get_number_of_lines(m_ex->register_text());

  if (lines >= 2)
  {
    m_ex->frame()->show_ex_message(
      wxString::Format(_("%d lines moved"), lines - 1).ToStdString());
  }

  return true;
}

void wex::addressrange::on_exit()
{
  delete m_process;
}
  
bool wex::addressrange::parse(
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
  if (!is_ok() || !m_begin.flags_supported(flags))
  {
    return false;
  }
  
  std::string text;
  
  for (auto i = m_begin.get_line() - 1; i < m_end.get_line(); i++)
  {
    char buffer[8];
    sprintf(buffer, "%6d ", i + 1);
    
    text += (flags.find("#") != std::string::npos ? buffer: std::string()) + 
      m_stc->GetLine(i);
  }
    
  m_ex->frame()->print_ex(m_ex, text);
  
  return true;
}
  
void wex::addressrange::set(address& begin, address& end, int lines)
{
  begin.set_line(m_stc->LineFromPosition(m_stc->GetCurrentPos()) + 1);
  end.set_line(begin.get_line() + lines - 1);
}

bool wex::addressrange::set_selection() const
{
  if (!m_stc->GetSelectedText().empty())
  {
    return true;
  }
  else if (!is_ok())
  {
    return false;
  }
  
  m_stc->SetSelection(
    m_stc->PositionFromLine(m_begin.get_line() - 1),
    m_stc->PositionFromLine(m_end.get_line()));

  return true;
}

bool wex::addressrange::sort(const std::string& parameters) const
{
  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !set_selection())
  {
    return false;
  }
  
  string_sort_t sort_t = 0;
  size_t pos = 0;
  size_t len = std::string::npos;

  if (m_stc->SelectionIsRectangle())
  {
    pos = m_stc->GetColumn(m_stc->GetSelectionStart());
    len = m_stc->GetColumn(m_stc->GetSelectionEnd() - pos);
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

  return sort_selection(m_stc, sort_t, pos, len);
}
  
bool wex::addressrange::substitute(const std::string& text, const char cmd)
{
  if (m_stc->GetReadOnly() || !is_ok())
  {
    return false;
  }
  
  std::string pattern;
  std::string repl;
  std::string options;
  
  switch (cmd)
  {
    case 's':
      if (!parse(text, pattern, repl, options))
      {
        return false;
      }
      break;
    case '&':
      repl = m_replacement;
      pattern = m_pattern;
      options = text;
      break;
    case '~':
      repl = m_replacement;
      pattern = m_pattern;
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

  auto searchFlags = m_ex->search_flags();
  if (options.find("i") != std::string::npos) searchFlags &= ~wxSTC_FIND_MATCHCASE;
    
  if ((searchFlags & wxSTC_FIND_REGEXP) && 
    pattern.size() == 2 && pattern.back() == '*' && repl.empty())
  {
    log::status("Replacement leads to infinite loop");
    return false;
  }
       
  if (!m_ex->marker_add('#', m_begin.get_line() - 1))
  {
    return false;
  }

  int corrected = 0;
  auto end_line = m_end.get_line() - 1;
  
  if (!m_stc->GetSelectedText().empty())
  {
    if (m_stc->GetLineSelEndPosition(end_line) == m_stc->PositionFromLine(end_line))
    {
      end_line--;
      corrected = 1;
    }
  }
  
  if (!m_ex->marker_add('$', end_line))
  {
    return false;
  }

  m_pattern = pattern;
  m_replacement = repl; 
  
  m_stc->set_search_flags(searchFlags);
  m_stc->BeginUndoAction();
  m_stc->SetTargetStart(m_stc->PositionFromLine(m_ex->marker_line('#')));
  m_stc->SetTargetEnd(m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  int nr_replacements = 0;
  int result = wxID_YES;
  const bool build = (repl.find_first_of("&0LU\\") != std::string::npos);
  const bool confirmed = (options.find("c") != std::string::npos);
  const bool global = (options.find("g") != std::string::npos);
  auto replacement(repl);

  while (m_stc->SearchInTarget(pattern) != -1 && result != wxID_CANCEL)
  {
    if (build)
    {
      replacement = build_replacement(repl);
    }
    
    if (confirmed)
    {
      result = confirm(pattern, replacement);
    }
        
    if (result == wxID_YES)
    {
      if (m_stc->is_hexmode())
      {  
        m_stc->get_hexmode().replace_target(replacement, false);
      }
      else
      {
        (searchFlags & wxSTC_FIND_REGEXP) ?
           m_stc->ReplaceTargetRE(replacement):
           m_stc->ReplaceTarget(replacement);
      }
        
      nr_replacements++;
    }
    
    m_stc->SetTargetStart(global ? 
      m_stc->GetTargetEnd():
      m_stc->GetLineEndPosition(m_stc->LineFromPosition(m_stc->GetTargetEnd())));
    m_stc->SetTargetEnd(m_stc->GetLineEndPosition(m_ex->marker_line('$')));
  
    if (m_stc->GetTargetStart() >= m_stc->GetTargetEnd())
    {
      break;
    }
  }
  
  if (m_stc->is_hexmode())
  {
    m_stc->get_hexmode().set_text(m_stc->get_hexmode().buffer());
  }

  m_stc->EndUndoAction();
  
  if (m_begin.m_address == "'<" && m_end.m_address == "'>")
  {
    m_stc->SetSelection(
      m_stc->PositionFromLine(m_ex->marker_line('#')),
      m_stc->PositionFromLine(m_ex->marker_line('$') + corrected));
  }

  m_ex->marker_delete('#');
  m_ex->marker_delete('$');
  
  m_ex->frame()->show_ex_message(wxString::Format(
    _("Replaced: %d occurrences of: %s"), nr_replacements, pattern.c_str()).ToStdString());

  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
  
  return true;
}

bool wex::addressrange::write(const std::string& text) const
{
  if (!set_selection())
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
    std::ios_base::app: std::ios::out).write(m_stc->get_selected_text());
}

bool wex::addressrange::yank(const char name) const
{
  return set_selection() && m_ex->yank(name);
}
