////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.cpp
// Purpose:   Implementation of class wex::addressrange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/common/cmdline.h>
#include <wex/common/util.h>
#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/core/temp-filename.h>
#include <wex/factory/process.h>
#include <wex/factory/sort.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/vi/addressrange.h>
#include <wex/vi/command-parser.h>
#include <wex/vi/ex-stream.h>
#include <wex/vi/ex.h>
#include <wex/vi/macros.h>
#include <wx/app.h>
#include <wx/msgdlg.h>

#include "global-env.h"

namespace wex
{
void convert_case(factory::stc* stc, std::string& target, char c)
{
  c == 'U' ? boost::algorithm::to_upper(target) :
             boost::algorithm::to_lower(target);

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
  set_range(range);
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
          convert_case(m_stc, target, c);
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
  return general(
    destination,
    [=, this]()
    {
      return yank();
    });
}

bool wex::addressrange::erase() const
{
  if (!m_stc->is_visual())
  {
    return m_ex->ex_stream()->erase(*this);
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
    if (auto expanded(command);
        !marker_and_register_expansion(m_ex, expanded) ||
        !shell_expansion(expanded))
    {
      return false;
    }
    else
    {
      return m_ex->frame()->process_async_system(
        expanded,
        m_stc->path().parent_path());
    }
  }

  if (!is_ok())
  {
    return false;
  }

  if (temp_filename tmp(true);
      m_stc->GetReadOnly() || m_stc->is_hexmode() || !write(tmp.name()))
  {
    return false;
  }
  else if (factory::process process;
           process.system(command + " " + tmp.name()) == 0)
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

bool wex::addressrange::general(
  const address&        destination,
  std::function<bool()> f) const
{
  const auto dest_line = destination.get_line();

  if (
    m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok() || dest_line == 0 ||
    (dest_line >= m_begin.get_line() && dest_line <= m_end.get_line()))
  {
    return false;
  }

  m_stc->BeginUndoAction();

  if (f())
  {
    m_stc->goto_line(dest_line - 1);
    m_stc->add_text(m_ex->register_text());
  }

  m_stc->EndUndoAction();

  return true;
}

bool wex::addressrange::global(const command_parser& cp) const
{
  if (!m_substitute.set_global(cp.command() + cp.text()))
  {
    return false;
  }

  if (m_substitute.is_clear())
  {
    m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);
    return true;
  }

  global_env g(this);

  if (!g.global(m_substitute))
  {
    return false;
  }

  if (g.hits() > 0)
  {
    if (g.has_commands())
      m_ex->frame()->show_ex_message(
        "executed: " + std::to_string(g.hits()) + " commands");
    else
      m_ex->frame()->show_ex_message(
        "found: " + std::to_string(g.hits()) + " matches");
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
    return m_ex->ex_stream()->join(*this);
  }

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !is_ok())
  {
    return false;
  }

  m_stc->BeginUndoAction();

  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_begin.get_line() - 1),
    m_stc->PositionFromLine(m_end.get_line() - 1));
  m_stc->LinesJoin();

  m_stc->EndUndoAction();

  return true;
}

bool wex::addressrange::move(const address& destination) const
{
  return general(
    destination,
    [=, this]()
    {
      return erase();
    });
}

bool wex::addressrange::parse(const command_parser& cp, info_message_t& im)
{
  if (!cp.range().empty())
  {
    set_range(cp.range());
  }

  im = info_message_t::NONE;

  switch (cp.command()[0])
  {
    case 0:
      return false;

    case 'c':
      if (cp.command() != "co" && cp.command() != "copy")
      {
        if (cp.text().find('|') != std::string::npos)
        {
          return change(after(cp.text(), '|'));
        }
        else
        {
          return m_ex->frame()->show_ex_input(m_ex->get_stc(), cp.command()[0]);
        }
      }
      [[fallthrough]];

    case 't':
      im = info_message_t::COPY;
      return copy(address(m_ex, cp.text()));

    case 'd':
      im = info_message_t::DEL;
      return erase();

    case 'g':
    case 'v':
      return global(cp);

    case 'j':
      return join();

    case 'm':
      im = info_message_t::MOVE;
      return move(address(m_ex, cp.text()));

    case 'l':
    case 'p':
      return (m_stc->GetName() != "Print" ? print(cp.text()) : false);

    case 's':
    case '&':
    case '~':
      return substitute(cp);

    case 'S':
      return sort(cp.text());

    case 'w':
      if (!cp.text().empty() && !cmdline::use_events())
      {
        const bool se = m_stc->GetSelectionEmpty();

        m_stc->position_save();

        const bool r = write(cp.text());

        m_stc->position_restore();

        if (se)
        {
          m_stc->SelectNone();
        }

        return r;
      }
      else
      {
        wxCommandEvent event(
          wxEVT_COMMAND_MENU_SELECTED,
          cp.text().empty() ? wxID_SAVE : wxID_SAVEAS);
        event.SetString(boost::algorithm::trim_left_copy(cp.text()));
        wxPostEvent(wxTheApp->GetTopWindow(), event);
        return true;
      }

    case 'y':
      im = info_message_t::YANK;
      return yank(cp.text().empty() ? '0' : static_cast<char>(cp.text()[0]));

    case '>':
      return shift_right();

    case '<':
      return shift_left();

    case '!':
      return escape(cp.text());

    case '@':
      return execute(cp.text());

    case '#':
    case 'n':
      return (m_stc->GetName() != "Print" ? print("#" + cp.text()) : false);

    default:
      log::status("Unknown range command") << cp.command();
      return false;
  }
}

bool wex::addressrange::print(const std::string& flags) const
{
  if (!is_ok() || !m_begin.flags_supported(flags))
  {
    return false;
  }

  m_ex->print(
    wex::write_lines(m_stc, m_begin.get_line() - 1, m_end.get_line(), flags));

  return true;
}

const std::string wex::addressrange::regex_commands() const
{
  // 2addr commands
  return std::string("(change|c\\b|"
                     "copy|co|t|"
                     "delete|d|"
                     "global|g|"
                     "join|j|"
                     "list|l|"
                     "move|m|"
                     "number|nu|"
                     "print|p|"
                     "substitute|s|"
                     "write|w|"
                     "yank|ya|"
                     "[Svy<>\\!&~@#])([\\s\\S]*)");
}

void wex::addressrange::set(int begin, int end)
{
  m_begin.set_line(begin);
  m_end.set_line(end);
}

void wex::addressrange::set(const std::string& begin, const std::string& end)
{
  m_begin.m_address = begin;

  if (const auto begin_line = m_begin.get_line(); begin_line > 0)
  {
    m_begin.set_line(begin_line);
  }

  m_end.m_address = end;

  if (const auto end_line = m_end.get_line(); end_line > 0)
  {
    m_end.set_line(end_line);
  }
}

void wex::addressrange::set(address& begin, address& end, int lines) const
{
  begin.set_line(m_stc->LineFromPosition(m_stc->GetCurrentPos()) + 1);
  end.set_line(begin.get_line() + lines - 1);
}

void wex::addressrange::set_range(const std::string& range)
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

  sort::sort_t sort_t = 0;

  size_t pos = 0, len = std::string::npos;

  if (m_stc->SelectionIsRectangle())
  {
    pos = m_stc->GetColumn(m_stc->GetSelectionStart());
    len = m_stc->GetColumn(m_stc->GetSelectionEnd() - pos);
  }

  if (!parameters.empty())
  {
    /// -u to sort unique lines
    /// -r to sort reversed (descending)
    if (
      (parameters[0] == '0') ||
      (!parameters.starts_with("u") && !parameters.starts_with("r") &&
       !isdigit(parameters[0])))
    {
      return false;
    }

    if (parameters.find("r") != std::string::npos)
      sort_t.set(sort::SORT_DESCENDING);
    if (parameters.find("u") != std::string::npos)
      sort_t.set(sort::SORT_UNIQUE);

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

  return wex::sort(sort_t, pos, len).selection(m_stc);
}

bool wex::addressrange::substitute(const command_parser& cp)
{
  if ((m_stc->is_visual() && m_stc->GetReadOnly()) || !is_ok())
  {
    return false;
  }

  data::substitute data(m_substitute);

  char        cmd = cp.command()[0];
  const auto& text(cp.text());

  switch (cmd)
  {
    case 's':
      if (!data.set(text))
      {
        return false;
      }
      break;

    case '&':
    case '~':
      data.set_options(text);
      break;

    default:
      log::debug("substitute unhandled command") << cmd;
      return false;
  }

  if (data.pattern().empty())
  {
    log::status("Pattern is empty");
    log::debug("substitute") << cmd << "empty pattern" << text;
    return false;
  }

  auto searchFlags = m_ex->search_flags();

  if (data.is_ignore_case())
    searchFlags &= ~wxSTC_FIND_MATCHCASE;

  if (
    (searchFlags & wxSTC_FIND_REGEXP) && data.pattern().size() == 2 &&
    data.pattern().back() == '*' && data.replacement().empty())
  {
    log::status("Replacement leads to infinite loop");
    return false;
  }

  if (!m_stc->is_visual())
  {
    return m_ex->ex_stream()->substitute(*this, data);
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

  m_substitute = data;

  m_stc->set_search_flags(searchFlags);
  m_stc->BeginUndoAction();
  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_ex->marker_line('#')),
    m_stc->GetLineEndPosition(m_ex->marker_line('$')));

  int        nr_replacements = 0;
  int        result          = wxID_YES;
  const bool build =
    (data.replacement().find_first_of("&0LU\\") != std::string::npos);
  auto replacement(data.replacement());

  while (m_stc->SearchInTarget(data.pattern()) != -1 && result != wxID_CANCEL)
  {
    if (build)
    {
      replacement = build_replacement(data.replacement());
    }

    if (data.is_confirmed())
    {
      result = confirm(data.pattern(), replacement);
    }

    if (result == wxID_YES)
    {
      if (m_stc->is_hexmode())
      {
        m_stc->get_hexmode_replace_target(replacement, false);
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
      data.is_global() ? m_stc->GetTargetEnd() :
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
    m_stc->get_hexmode_sync();
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
    " occurrences of: " + data.pattern());

  m_stc->IndicatorClearRange(0, m_stc->GetTextLength() - 1);

  return true;
}

bool wex::addressrange::write(const std::string& text) const
{
  if (!set_selection())
  {
    return false;
  }

  auto filename(boost::algorithm::trim_left_copy(
    text.find(">>") != std::string::npos ? wex::after(text, '>', false) :
                                           text));

#ifdef __UNIX__
  if (filename.find("~") != std::string::npos)
  {
    filename.replace(filename.find("~"), 1, wxGetHomeDir());
  }
#endif

  if (!m_stc->is_visual())
  {
    return m_ex->ex_stream()->write(
      *this,
      filename,
      text.find(">>") != std::string::npos);
  }
  else
  {
    return wex::file(
             path(filename),
             text.find(">>") != std::string::npos ?
               std::ios::out | std::ios_base::app :
               std::ios::out)
      .write(m_stc->get_selected_text());
  }
}

bool wex::addressrange::yank(char name) const
{
  if (!m_stc->is_visual())
  {
    return m_ex->ex_stream()->yank(*this, name);
  }
  else
  {
    return set_selection() && m_ex->yank(name);
  }
}
