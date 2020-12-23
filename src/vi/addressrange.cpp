////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.cpp
// Purpose:   Implementation of class wex::addressrange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/addressrange.h>
#include <wex/core.h>
#include <wex/ex-stream.h>
#include <wex/ex.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/managed-frame.h>
#include <wex/process.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

namespace wex
{
  class global_env
  {
  public:
    global_env(const addressrange* ar, const std::string& commands)
      : m_ex(ar->m_ex)
      , m_ar(ar)
    {
      m_ex->get_stc()->set_search_flags(m_ex->search_flags());
      m_ex->get_stc()->BeginUndoAction();

      for (tokenizer tkz(commands, "|"); tkz.has_more_tokens();)
      {
        // Prevent recursive global.
        if (const auto cmd(tkz.get_next_token());
            cmd[0] != 'g' && cmd[0] != 'v')
        {
          if (cmd[0] == 'd' || cmd[0] == 'm')
          {
            m_changes++;
          }

          m_commands.emplace_back(cmd);
        }
      }
    }

    ~global_env()
    {
      m_ex->get_stc()->EndUndoAction();
      m_ex->marker_delete('%');
    }

    auto changes() const { return m_changes; };

    bool commands() const { return !m_commands.empty(); };

    bool for_each(int line) const
    {
      if (!commands())
      {
        m_ex->get_stc()->set_indicator(
          m_ar->m_find_indicator,
          m_ex->get_stc()->GetTargetStart(),
          m_ex->get_stc()->GetTargetEnd());
      }
      else
      {
        for (const auto& it : m_commands)
        {
          if (!m_ex->command(":" + std::to_string(line + 1) + it))
          {
            m_ex->frame()->show_ex_message(
              m_ex->get_command().command() + " failed");
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
        for (int i = start;
             i < end && i < m_ex->get_stc()->get_line_count() - 1;)
        {
          if (commands())
          {
            if (!for_each(i))
              return false;
          }
          else
          {
            m_ex->get_stc()->set_indicator(
              m_ar->m_find_indicator,
              m_ex->get_stc()->PositionFromLine(i),
              m_ex->get_stc()->GetLineEndPosition(i));
          }

          if (m_changes == 0)
          {
            i++;
          }
          else
          {
            end -= m_changes;
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
    const addressrange*      m_ar;
    std::vector<std::string> m_commands;
    int                      m_changes{0};
    ex*                      m_ex;
  };

  void toggle_case(stc* stc, std::string& target, char c)
  {
    std::transform(
      target.begin(),
      target.end(),
      target.begin(),
      c == 'U' ? ::toupper : ::tolower);

    stc->Replace(stc->GetTargetStart(), stc->GetTargetEnd(), target);
  }
}; // namespace wex

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

const std::string
wex::addressrange::build_replacement(const std::string& text) const
{
  if (
    text.find("&") == std::string::npos && text.find("\0") == std::string::npos)
  {
    return text;
  }

  std::string target(
    m_stc->GetTextRange(m_stc->GetTargetStart(), m_stc->GetTargetEnd())),
    replacement;

  bool backslash = false;

  for (const auto& c : text)
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
      case 'U':
        if (backslash)
        {
          toggle_case(m_stc, target, c);
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

  m_stc->add_text(text);

  return true;
}

int wex::addressrange::confirm(
  const std::string& pattern,
  const std::string& replacement) const
{
  wxMessageDialog msgDialog(
    m_stc,
    _("Replace") + " " + pattern + " " + _("with") + " " + replacement,
    _("Replace"),
    wxCANCEL | wxYES_NO);

  const auto line = m_stc->LineFromPosition(m_stc->GetTargetStart());

  msgDialog.SetExtendedMessage(
    "Line " + std::to_string(line + 1) + ": " + m_stc->GetLineText(line));

  m_stc->goto_line(line);
  m_stc->set_indicator(
    m_find_indicator,
    m_stc->GetTargetStart(),
    m_stc->GetTargetEnd());

  return msgDialog.ShowModal();
}

bool wex::addressrange::copy(const wex::address& destination) const
{
  const auto dest_line = destination.get_line();

  if (
    m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() || dest_line == 0 ||
    (dest_line >= m_begin.get_line() && dest_line <= m_end.get_line()))
  {
    return false;
  }

  m_stc->BeginUndoAction();

  if (yank())
  {
    m_stc->goto_line(dest_line - 1);
    m_stc->add_text(m_ex->register_text());
  }

  m_stc->EndUndoAction();

  return true;
}

bool wex::addressrange::erase() const
{
  if (!m_stc->is_visual())
  {
    return m_stc->get_file().ex_stream()->erase(*this);
  }

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !set_selection())
  {
    return false;
  }

  m_ex->cut();

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
      !shell_expansion(expanded))
      return false;

    if (m_process == nullptr)
    {
      m_process = new wex::process();
    }
    else if (m_process->is_running())
    {
      log::trace("escape") << command << "while running"
                           << m_process->get_exec();
    }

    return m_process->execute(
      expanded,
      process::EXEC_NO_WAIT,
      m_stc->get_filename().get_path());
  }

  if (!is_ok())
  {
    return false;
  }

  const std::string tmp_filename(
    path(
      std::filesystem::temp_directory_path().string(),
      std::to_string(std::time(nullptr)))
      .data()
      .string());

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !write(tmp_filename))
  {
    return false;
  }

  wex::process process;

  const bool ok =
    process.execute(command + " " + tmp_filename, process::EXEC_WAIT);

  if (remove(tmp_filename.c_str()) != 0)
  {
    log::status("Could not remove file");
  }

  if (ok)
  {
    if (!process.get_stdout().empty())
    {
      m_stc->BeginUndoAction();

      if (erase())
      {
        m_stc->add_text(process.get_stdout());
      }

      m_stc->EndUndoAction();

      return true;
    }
    else if (!process.get_stderr().empty())
    {
      m_ex->frame()->show_ex_message(process.get_stderr());
      log("escape") << process.get_stderr();
    }
  }

  return false;
}

bool wex::addressrange::execute(const std::string& reg) const
{
  if (!is_ok() || !ex::get_macros().is_recorded_macro(reg))
  {
    return false;
  }

  bool error = false;

  m_stc->BeginUndoAction();

  for (auto i = m_begin.get_line() - 1; i < m_end.get_line() && !error; i++)
  {
    if (!m_ex->command("@" + reg))
    {
      error = true;
    }
  }

  m_stc->EndUndoAction();

  return !error;
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

  const auto& pattern = next.get_next_token();
  std::string rest;

  if (next.has_more_tokens())
  {
    if (const auto token(next.get_next_token()); !token.empty())
    {
      const auto command = token[0];
      auto       arg(token.size() > 1 ? token.substr(1) : std::string());

      if (next.has_more_tokens())
      {
        auto subpattern = next.get_next_token();

        if (subpattern.empty())
        {
          subpattern = pattern;
        }

        arg += "/" + subpattern + "/" + next.get_string();
      }

      rest = command + arg;
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

  const global_env g(this, rest);
  m_ex->marker_add('%', m_end.get_line() - 1);
  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_begin.get_line() - 1),
    m_stc->GetLineEndPosition(m_ex->marker_line('%')));

  const bool infinite =
    (g.changes() > 0 && rest != "$" && rest != "1" && rest != "d");
  int hits  = 0;
  int start = 0;

  while (m_stc->SearchInTarget(pattern) != -1)
  {
    auto match = m_stc->LineFromPosition(m_stc->GetTargetStart());

    if (!inverse)
    {
      if (!g.for_each(match))
        return false;
      hits++;
    }
    else
    {
      if (!g.for_each(start, match, hits))
        return false;
      start = match + 1;
    }

    if (hits > 50 && infinite)
    {
      m_ex->frame()->show_ex_message(
        "possible infinite loop at " + std::to_string(match));
      return false;
    }

    m_stc->SetTargetRange(
      g.changes() > 0 ? m_stc->PositionFromLine(match) : m_stc->GetTargetEnd(),
      m_stc->GetLineEndPosition(m_ex->marker_line('%')));

    if (m_stc->GetTargetStart() >= m_stc->GetTargetEnd())
    {
      break;
    }
  }

  if (inverse)
  {
    if (auto match = m_stc->get_line_count(); !g.for_each(start, match, hits))
    {
      return false;
    }
  }

  if (hits > 0)
  {
    if (g.commands())
      m_ex->frame()->show_ex_message(
        "executed: " + std::to_string(hits) + " commands");
    else
      m_ex->frame()->show_ex_message(
        "found: " + std::to_string(hits) + " matches");
  }

  return true;
}

bool wex::addressrange::indent(bool forward) const
{
  if (
    m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() || !set_selection())
  {
    return false;
  }

  m_stc->BeginUndoAction();
  m_stc->SendMsg(forward ? wxSTC_CMD_TAB : wxSTC_CMD_BACKTAB);
  m_stc->EndUndoAction();

  return true;
}

bool wex::addressrange::is_ok() const
{
  return m_begin.get_line() > 0 && m_end.get_line() > 0 &&
         m_begin.get_line() <= m_end.get_line();
}

bool wex::addressrange::join() const
{
  if (!m_stc->is_visual())
  {
    return m_stc->get_file().ex_stream()->join(*this);
  }

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok())
  {
    return false;
  }

  m_stc->BeginUndoAction();
  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_begin.get_line() - 1),
    m_stc->PositionFromLine(m_end.get_line()));
  m_stc->LinesJoin();
  m_stc->EndUndoAction();

  return true;
}

bool wex::addressrange::move(const address& destination) const
{
  const auto dest_line = destination.get_line();

  if (
    m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() || dest_line == 0 ||
    (dest_line >= m_begin.get_line() && dest_line <= m_end.get_line()))
  {
    return false;
  }

  m_stc->BeginUndoAction();

  if (erase())
  {
    m_stc->goto_line(dest_line - 1);
    m_stc->add_text(m_ex->register_text());
  }

  m_stc->EndUndoAction();

  return true;
}

void wex::addressrange::on_exit()
{
  delete m_process;
}

bool wex::addressrange::parse(
  const std::string& command,
  const std::string& text,
  info_message_t&    im)
{
  im = info_message_t::NONE;

  switch (command[0])
  {
    case 0:
      return false;

    case 'c':
      if (text.find('|') != std::string::npos)
      {
        return change(after(text, '|'));
      }
      else
      {
        return m_ex->frame()->show_ex_input(m_ex, command[0]);
      }

    case 'd':
      im = info_message_t::DEL;
      return erase();

    case 'v':
    case 'g':
      return global(text, command[0] == 'v');

    case 'j':
      return join();

    case 'm':
      im = info_message_t::MOVE;
      return move(address(m_ex, text));

    case 'l':
    case 'p':
      return (m_stc->GetName() != "Print" ? print(text) : false);

    case 's':
    case '&':
    case '~':
      return substitute(text, command[0]);

    case 'S':
      return sort(text);

    case 't':
      im = info_message_t::COPY;
      return copy(address(m_ex, text));

    case 'w':
      if (!text.empty())
      {
        return write(text);
      }
      else
      {
        wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, wxID_SAVE);
        wxPostEvent(wxTheApp->GetTopWindow(), event);
        return true;
      }

    case 'y':
      im = info_message_t::YANK;
      return yank(text.empty() ? '0' : (char)text[0]);

    case '>':
      return shift_right();

    case '<':
      return shift_left();

    case '!':
      return escape(text);

    case '@':
      return execute(text);

    case '#':
      return (m_stc->GetName() != "Print" ? print("#" + text) : false);

    default:
      log::status("Unknown range command") << command;
      return false;
  }
}

bool wex::addressrange::parse(
  const std::string& command_org,
  std::string&       pattern,
  std::string&       replacement,
  std::string&       options) const
{
  // If there are escaped / chars in the text,
  // temporarily replace them to an unused char, so
  // we can use string tokenizer with / as separator.
  bool escaped = false;

  auto command(command_org);

  if (
    command.find("\\\\/") == std::string::npos &&
    command.find("\\/") != std::string::npos)
  {
    if (command.find(char(1)) == std::string::npos)
    {
      replace_all(command, "\\/", "\x01");
      escaped = true;
    }
    else
    {
      log::debug("internal char exists") << command;
      return false;
    }
  }

  if (std::vector<std::string> v;
      match("/(.*)/(.*)/([cgi]*)", command, v) == 3 ||
      match("/(.*)/(.*)", command, v) == 2 || match("/(.*)", command, v) == 1)
  {
    pattern = v[0];
    if (v.size() >= 2)
      replacement = v[1];
    if (v.size() >= 3)
      options = v[2];

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

    text += (flags.find("#") != std::string::npos ? buffer : std::string()) +
            m_stc->GetLine(i);
  }

  m_ex->frame()->print_ex(m_ex, text);

  return true;
}

const std::string wex::addressrange::regex_commands() const
{
  // 2addr commands
  return std::string("(change\\b|"
                     "copy\\b|co\\b|"
                     "delete\\b|"
                     "global\\b|"
                     "join\\b|"
                     "list\\b|"
                     "move\\b|"
                     "number\\b|nu\\b|"
                     "print\\b|"
                     "substitute\\b|"
                     "write\\b|"
                     "yank\\b|ya\\b|"
                     "[cdgjlmpsStvwy<>\\!&~@#])([\\s\\S]*)");
}

void wex::addressrange::set(address& begin, address& end, int lines) const
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

  size_t pos = 0, len = std::string::npos;

  if (m_stc->SelectionIsRectangle())
  {
    pos = m_stc->GetColumn(m_stc->GetSelectionStart());
    len = m_stc->GetColumn(m_stc->GetSelectionEnd() - pos);
  }

  if (!parameters.empty())
  {
    if (
      (parameters[0] == '0') ||
      (parameters.find("u") != 0 && parameters.find("r") != 0 &&
       !isdigit(parameters[0])))
    {
      return false;
    }

    if (parameters.find("r") != std::string::npos)
      sort_t.set(STRING_SORT_DESCENDING);
    if (parameters.find("u") != std::string::npos)
      sort_t.set(STRING_SORT_UNIQUE);

    if (isdigit(parameters[0]))
    {
      try
      {
        pos = (std::stoi(parameters) > 0 ? std::stoi(parameters) - 1 : 0);

        if (parameters.find(",") != std::string::npos)
        {
          len =
            std::stoi(parameters.substr(parameters.find(',') + 1)) - pos + 1;
        }
      }
      catch (...)
      {
      }
    }
  }

  return sort_selection(m_stc, sort_t, pos, len);
}

bool wex::addressrange::substitute(const std::string& text, char cmd)
{
  if ((m_stc->is_visual() && m_stc->GetReadOnly()) || !is_ok())
  {
    return false;
  }

  std::string pattern, repl, options;

  switch (cmd)
  {
    case 's':
      if (!parse(text, pattern, repl, options))
      {
        return false;
      }
      break;

    case '&':
      pattern = m_pattern;
      repl    = m_replacement;
      options = text;
      break;

    case '~':
      pattern = find_replace_data::get()->get_find_string();
      repl    = m_replacement;
      options = text;
      break;

    default:
      log::debug("substitute unhandled command") << cmd;
      return false;
  }

  if (pattern.empty())
  {
    log::status("Pattern is empty");
    log::debug("substitute empty pattern") << cmd;
    return false;
  }

  auto searchFlags = m_ex->search_flags();
  if (options.find("i") != std::string::npos)
    searchFlags &= ~wxSTC_FIND_MATCHCASE;

  if (
    (searchFlags & wxSTC_FIND_REGEXP) && pattern.size() == 2 &&
    pattern.back() == '*' && repl.empty())
  {
    log::status("Replacement leads to infinite loop");
    return false;
  }

  if (!m_stc->is_visual())
  {
    return m_stc->get_file().ex_stream()->substitute(*this, pattern, repl);
  }

  if (!m_ex->marker_add('#', m_begin.get_line() - 1))
  {
    log::debug("substitute could not add marker");
    return false;
  }

  int  corrected = 0;
  auto end_line  = m_end.get_line() - 1;

  if (!m_stc->GetSelectedText().empty())
  {
    if (
      m_stc->GetLineSelEndPosition(end_line) ==
      m_stc->PositionFromLine(end_line))
    {
      end_line--;
      corrected = 1;
    }
  }

  if (!m_ex->marker_add('$', end_line))
  {
    log::debug("substitute could not add marker");
    return false;
  }

  m_pattern     = pattern;
  m_replacement = repl;

  m_stc->set_search_flags(searchFlags);
  m_stc->BeginUndoAction();
  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_ex->marker_line('#')),
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  int        nr_replacements = 0;
  int        result          = wxID_YES;
  const bool build     = (repl.find_first_of("&0LU\\") != std::string::npos);
  const bool confirmed = (options.find("c") != std::string::npos);
  const bool global    = (options.find("g") != std::string::npos);
  auto       replacement(repl);

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
          m_stc->ReplaceTargetRE(replacement) :
          m_stc->ReplaceTarget(replacement);
      }

      nr_replacements++;
    }

    m_stc->SetTargetRange(
      global ? m_stc->GetTargetEnd() :
               m_stc->GetLineEndPosition(
                 m_stc->LineFromPosition(m_stc->GetTargetEnd())),
      m_stc->GetLineEndPosition(m_ex->marker_line('$')));

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

  m_ex->frame()->show_ex_message(
    "Replaced: " + std::to_string(nr_replacements) +
    " occurrences of: " + pattern);

  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);

  return true;
}

bool wex::addressrange::write(const std::string& text) const
{
  if (!set_selection())
  {
    return false;
  }

  auto filename(trim(
    text.find(">>") != std::string::npos ? wex::after(text, '>', false) : text,
    skip_t().set(TRIM_LEFT)));

#ifdef __UNIX__
  if (filename.find("~") != std::string::npos)
  {
    filename.replace(filename.find("~"), 1, wxGetHomeDir());
  }
#endif

  return wex::file(
           filename,
           text.find(">>") != std::string::npos ?
             std::ios::out | std::ios_base::app :
             std::ios::out)
    .write(m_stc->get_selected_text());
}

bool wex::addressrange::yank(char name) const
{
  return set_selection() && m_ex->yank(name);
}
