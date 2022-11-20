////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.cpp
// Purpose:   Implementation of class wex::addressrange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/common/util.h>
#include <wex/core/cmdline.h>
#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/command-parser.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>
#include <wex/factory/process.h>
#include <wex/factory/sort.h>
#include <wex/factory/stc-undo.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wx/app.h>
#include <wx/msgdlg.h>

#include "addressrange-mark.h"
#include "global-env.h"

namespace wex
{
void convert_case(factory::stc* stc, std::string& target, char c)
{
  c == 'U' ? boost::algorithm::to_upper(target) :
             boost::algorithm::to_lower(target);

  stc->Replace(stc->GetTargetStart(), stc->GetTargetEnd(), target);
}

bool prep(data::substitute& data, int searchFlags, const command_parser& cp)
{
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

  if (
    (searchFlags & wxSTC_FIND_REGEXP) && data.pattern().size() == 2 &&
    data.pattern().back() == '*' && data.replacement().empty())
  {
    log::status("Replacement leads to infinite loop");
    return false;
  }

  return true;
}
}; // namespace wex

wex::addressrange::addressrange(ex* ex, int lines)
  : m_begin(ex)
  , m_end(ex)
  , m_ex(ex)
  , m_stc(ex->get_stc())
  , m_commands(init_commands())
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

wex::addressrange::addressrange(ex* ex, const std::string& range)
  : m_begin(ex)
  , m_end(ex)
  , m_ex(ex)
  , m_stc(ex->get_stc())
  , m_commands(init_commands())
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
  m_stc->set_indicator(m_find_indicator);

  return msgDialog.ShowModal();
}

bool wex::addressrange::copy(const command_parser& cp)
{
  if (cp.command() != "co" && cp.command() != "copy")
  {
    if (cp.text().find('|') != std::string::npos)
    {
      return change(find_after(cp.text(), "|"));
    }
    else
    {
      return m_ex->frame()->show_ex_input(m_ex->get_stc(), cp.command()[0]);
    }
  }

  return false;
}

bool wex::addressrange::copy(const address& destination) const
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
  /// Filters range with command.
  /// The address range is used as input for the command,
  /// and the output of the command replaces the address range.
  /// For example: addressrange(96, 99).escape("sort")
  /// or (ex command::96,99!sort)
  /// will pass lines 96 through 99 through the sort filter and
  /// replace those lines with the output of sort.
  /// If you did not specify an address range,
  /// the command is run as an asynchronous process.

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
        process_data(expanded).start_dir(m_stc->path().parent_path()));
    }
  }

  if (!is_ok())
  {
    return false;
  }

  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !set_selection())
  {
    return false;
  }
  else if (factory::process process;
           process.system(wex::process_data(command).std_in(
             m_stc->get_selected_text())) == 0)
  {
    if (const auto& out(process.std_out()); !out.empty())
    {
      stc_undo undo(m_stc);

      if (erase())
      {
        m_stc->add_text(out);
      }

      return true;
    }
    else if (const auto err(process.std_err()); !err.empty())
    {
      m_ex->frame()->show_ex_message(err);
      log("escape") << err;
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

  stc_undo undo(m_stc);

  for (auto i = m_begin.get_line() - 1; i < m_end.get_line() && !error; i++)
  {
    if (!m_ex->command("@" + reg))
    {
      error = true;
    }
  }

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

  stc_undo ndo(m_stc);

  if (f())
  {
    m_stc->goto_line(dest_line - 1);
    m_stc->add_text(m_ex->register_text());
  }

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

  /// Performs the global command (g) on this range.
  /// normally performs command on each match, if inverse
  /// performs (v) command if line does not match

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

  stc_undo undo(m_stc);
  m_stc->SendMsg(forward ? wxSTC_CMD_TAB : wxSTC_CMD_BACKTAB);

  return true;
}

const wex::addressrange::commands_t wex::addressrange::init_commands()
{
  return {
    {"c",
     [&](const command_parser& cp, info_message_t& msg)
     {
       if (copy(cp))
       {
         return true;
       }
       msg = info_message_t::COPY;
       return copy(address(m_ex, cp.text()));
     }},
    {"d",
     [&](const command_parser& cp, info_message_t& msg)
     {
       msg = info_message_t::DEL;
       return erase();
     }},
    {"gv",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return global(cp);
     }},
    {"j",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return join();
     }},
    {"lp#n",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return print(cp);
     }},
    {"m",
     [&](const command_parser& cp, info_message_t& msg)
     {
       msg = info_message_t::MOVE;
       return move(address(m_ex, cp.text()));
     }},
    {"s&~",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return substitute(cp);
     }},
    {"S",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return sort(cp.text());
     }},
    {"t",
     [&](const command_parser& cp, info_message_t& msg)
     {
       msg = info_message_t::COPY;
       return copy(address(m_ex, cp.text()));
     }},
    {"w",
     [&](const command_parser& cp, info_message_t& msg)
     {
       const bool result = write(cp);
       if (cp.command() == "wq")
       {
         POST_CLOSE(
           wxEVT_CLOSE_WINDOW,
           cp.text().find("!") == std::string::npos)
       }
       return result;
     }},
    {"x",
     [&](const command_parser& cp, info_message_t& msg)
     {
       write(cp);
       POST_CLOSE(wxEVT_CLOSE_WINDOW, cp.text().find("!") == std::string::npos)
       return true;
     }},
    {"y",
     [&](const command_parser& cp, info_message_t& msg)
     {
       msg = info_message_t::YANK;
       return yank(cp);
     }},
    {">",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return shift_right();
     }},
    {"<",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return shift_left();
     }},
    {"!",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return escape(cp.text());
     }},
    {"@",
     [&](const command_parser& cp, info_message_t& msg)
     {
       return execute(cp.text());
     }}};
}

bool wex::addressrange::is_ok() const
{
  return m_begin.get_line() > 0 && m_end.get_line() > 0 &&
         m_begin.get_line() <= m_end.get_line();
}

bool wex::addressrange::is_selection() const
{
  return (m_begin.m_address + "," + m_end.m_address) ==
         ex_command::selection_range();
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

  stc_undo undo(m_stc);

  m_stc->SetTargetRange(
    m_stc->PositionFromLine(m_begin.get_line() - 1),
    m_stc->PositionFromLine(m_end.get_line() - 1));
  m_stc->LinesJoin();

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
  if (!cp.range().empty() && !set_range(cp.range()))
  {
    return false;
  }

  im = info_message_t::NONE;

  if (const auto& it = std::find_if(
        m_commands.begin(),
        m_commands.end(),
        [&](auto const& e)
        {
          return std::any_of(
            e.first.begin(),
            e.first.end(),
            [cp](const auto& p)
            {
              return p == cp.command()[0];
            });
        });
      it != m_commands.end())
  {
    return it->second(cp, im);
  }
  else
  {
    log::status("Unknown range command") << cp.command();
    return false;
  }
}

bool wex::addressrange::print(const command_parser& cp)
{
  const std::string arg(
    cp.command()[0] == '#' || cp.command()[0] == 'n' ? "#" : std::string());

  return (m_stc->GetName() != "Print" ? print(arg + cp.text()) : false);
}

bool wex::addressrange::print(const std::string& flags) const
{
  if (!is_ok() || !address::flags_supported(flags))
  {
    return false;
  }

  m_ex->print(
    get_lines(m_stc, m_begin.get_line() - 1, m_end.get_line(), flags));

  return true;
}

const std::string wex::addressrange::regex_commands() const
{
  // 2addr commands
  return "(change\\b|c\\b|"
         "copy|co|t|"
         "delete\\b|d\\b|"
         "global\\b|g\\b|"
         "join\\b|j\\b|"
         "list\\b|l\\b|"
         "move|m|"
         "number\\b|nu\\b|"
         "print\\b|p\\b|"
         "substitute\\b|s\\b|"
         "write\\b|wq|w\\b|"
         "xit\\b|x\\b|"
         "yank\\b|ya\\b|"
         "[Sv<>\\!&~@#])([\\s\\S]*)";
}

void wex::addressrange::set(int begin, int end)
{
  m_begin.m_type = address::IS_BEGIN;
  m_end.m_type   = address::IS_END;

  m_begin.set_line(begin);
  m_end.set_line(end);
}

bool wex::addressrange::set(const std::string& begin, const std::string& end)
{
  m_begin.m_type = address::IS_BEGIN;
  m_end.m_type   = address::IS_END;

  return set_single(begin, m_begin) && set_single(end, m_end);
}

void wex::addressrange::set(address& begin, address& end, int lines) const
{
  begin.m_type = address::IS_BEGIN;
  end.m_type   = address::IS_END;

  begin.set_line(m_stc->LineFromPosition(m_stc->GetCurrentPos()) + 1);
  end.set_line(begin.get_line() + lines - 1);
}

bool wex::addressrange::set_range(const std::string& range)
{
  if (range == "%")
  {
    return set("1", "$");
  }
  else if (range == "*")
  {
    set(
      m_stc->GetFirstVisibleLine() + 1,
      m_stc->GetFirstVisibleLine() + m_stc->LinesOnScreen() + 1);
    return true;
  }
  else if (const auto comma(range.find(",")); comma != std::string::npos)
  {
    return set(range.substr(0, comma), range.substr(comma + 1));
  }
  else
  {
    return set(range, range);
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
  else
  {
    m_stc->SetSelection(
      m_stc->PositionFromLine(m_begin.get_line() - 1),
      m_stc->PositionFromLine(m_end.get_line()));
    return true;
  }
}

bool wex::addressrange::set_single(const std::string& line, address& addr)
{
  addr.m_address = line;

  if (const auto line_no = addr.get_line(
        addr.type() == address::IS_BEGIN ? m_stc->GetCurrentPos() :
                                           m_stc->GetTargetEnd());
      line_no > 0)
  {
    addr.set_line(line_no);
    return true;
  }
  else
  {
    log::debug("addressrange line") << line;
    return false;
  }
}

bool wex::addressrange::sort(const std::string& parameters) const
{
  if (m_stc->GetReadOnly() || m_stc->is_hexmode() || !set_selection())
  {
    return false;
  }

  /// Sorts range, with optional parameters:
  /// -u to sort unique lines
  /// -r to sort reversed (descending)
  ///  - x,y sorts rectangle within range: x start col, y end col (exclusive).

  factory::sort::sort_t sort_t = 0;

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
      sort_t.set(factory::sort::SORT_DESCENDING);
    if (parameters.find("u") != std::string::npos)
      sort_t.set(factory::sort::SORT_UNIQUE);

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

  return factory::sort(sort_t, pos, len).selection(m_stc);
}

bool wex::addressrange::substitute(const command_parser& cp)
{
  data::substitute data(m_substitute);
  auto             searchFlags = m_ex->search_flags();

  /// Substitutes range.
  /// text format: /pattern/replacement/options
  /// Pattern might contain:
  /// - $ to match a line end
  /// Replacement might contain:
  /// - & or \\0 to represent the target in the replacement
  /// - \\U to convert target to uppercase
  /// - \\L to convert target to lowercase
  /// Options can be:
  /// - c : Ask for confirm
  /// - i : Case insensitive
  /// - g : Do global on line, without this flag replace first match only
  /// e.g. /$/EOL appends the string EOL at the end of each line.
  /// Merging is not yet possible using a \n target,
  /// you can create a macro for that.
  /// cmd is one of s, & or ~
  /// - s : default, normal substitute
  /// - & : repeat last substitute (text contains options)
  /// - ~ : repeat last substitute with pattern from find replace data
  ///      (text contains options)

  if (!is_ok() || !prep(data, searchFlags, cp))
  {
    return false;
  }

  if (!m_stc->is_visual())
  {
    return m_ex->ex_stream()->substitute(*this, data);
  }
  else if (m_stc->GetReadOnly())
  {
    return false;
  }

  if (data.is_ignore_case())
    searchFlags &= ~wxSTC_FIND_MATCHCASE;

  addressrange_mark am(*this, data);

  if (!am.set())
  {
    log::debug("substitute could not set marker");
    return false;
  }

  m_substitute = data;
  m_stc->set_search_flags(searchFlags);

  int        nr_replacements = 0;
  int        result          = wxID_YES;
  const bool do_build =
    (data.replacement().find_first_of("&0LU\\") != std::string::npos);
  auto replacement(data.replacement());

  while (am.search(data) && result != wxID_CANCEL)
  {
    if (do_build)
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

    if (!am.update())
    {
      break;
    }
  }

  am.end();

  m_ex->frame()->show_ex_message(
    "Replaced: " + std::to_string(nr_replacements) +
    " occurrences of: " + data.pattern());

  return true;
}

bool wex::addressrange::write(const command_parser& cp)
{
  if (!cp.text().empty() && !cmdline::use_events())
  {
    stc_undo undo(
      m_stc,
      stc_undo::undo_t().set(stc_undo::UNDO_POS).set(stc_undo::UNDO_SEL_NONE));
    return write(cp.text());
  }
  else if (!m_stc->is_visual())
  {
    return m_ex->ex_stream()->write();
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
}

bool wex::addressrange::write(const std::string& text) const
{
  if (!set_selection())
  {
    return false;
  }

  auto filename(boost::algorithm::trim_left_copy(
    text.find(">>") != std::string::npos ? rfind_after(text, ">") : text));

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
    return file(
             path(filename),
             text.find(">>") != std::string::npos ?
               std::ios::out | std::ios_base::app :
               std::ios::out)
      .write(m_stc->get_selected_text());
  }
}

bool wex::addressrange::yank(const command_parser& cp)
{
  return yank(cp.text().empty() ? '0' : static_cast<char>(cp.text().back()));
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
