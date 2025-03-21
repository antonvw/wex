////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <charconv>

#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/ex/address.h>
#include <wex/ex/addressrange.h>
#include <wex/ex/command-parser.h>
#include <wex/ex/ex-stream.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/factory/process.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

#define SEARCH_TARGET                                                          \
  if (ex->get_stc()->SearchInTarget(text) != -1)                               \
  {                                                                            \
    return ex->get_stc()->LineFromPosition(ex->get_stc()->GetTargetStart()) +  \
           1;                                                                  \
  }

namespace wex
{
int find_stc(ex* ex, const std::string& text, int start_pos, bool forward)
{
  ex->get_stc()->SetTargetRange(
    start_pos,
    forward ? ex->get_stc()->GetTextLength() : 0);

  SEARCH_TARGET;

  ex->get_stc()->SetTargetRange(
    forward ? 0 : ex->get_stc()->GetTextLength(),
    start_pos);

  SEARCH_TARGET;

  return 0;
}

int find_stream(ex* ex, const std::string& text, bool forward)
{
  return ex->ex_stream()->find(text, 0, forward) ?
           ex->ex_stream()->get_current_line() + 1 :
           0;
}
}; // namespace wex

enum class wex::address::add_t
{
  APPEND,
  INSERT
};

wex::address::address(ex* ex, int line)
  : m_ex(ex)
  , m_line(line)
{
}

wex::address::address(ex* ex, const std::string& address)
  : m_address(address)
  , m_ex(ex)
{
}

bool wex::address::add(add_t type, const std::string& text) const
{
  const auto line = get_line();
  if (line <= 0)
  {
    return false;
  }

  if (!m_ex->get_stc()->is_visual())
  {
    m_ex->ex_stream()->insert_text(
      m_line,
      text + m_ex->get_stc()->eol(),
      type == add_t::APPEND ? ex_stream::loc_t::AFTER :
                              ex_stream::loc_t::BEFORE);
    return true;
  }

  if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode())
  {
    return false;
  }
  m_ex->get_stc()->insert_text(
    m_ex->get_stc()->PositionFromLine(type == add_t::APPEND ? line : line - 1),
    text + m_ex->get_stc()->eol());

  if (type == add_t::APPEND)
  {
    m_ex->get_stc()->goto_line(line + get_number_of_lines(text) - 1);
  }

  return true;
}

bool wex::address::adjust_window(const std::string& text) const
{
  regex v("([-+=.^]*)([0-9]+)?(.*)");

  if (v.match(text) != 3)
  {
    return false;
  }

  int count = 2;
  std::from_chars(v[1].data(), v[1].data() + v[1].size(), count);

  const auto& flags(v[2]);

  if (!flags_supported(flags))
  {
    return false;
  }

  int  begin     = get_line();
  bool separator = false;

  if (const auto type(v[0]); !type.empty())
  {
    switch (static_cast<int>(type.at(0)))
    {
      case '-':
        begin -= ((type.length() * count) - 1);
        break;
      case '+':
        begin += (((type.length() - 1) * count) + 1);
        break;
      case '^':
        begin += (((type.length() + 1) * count) - 1);
        break;
      case '=':
      case '.':
        if (count == 0)
        {
          return false;
        }
        separator = (type.at(0) == '=');
        begin -= (count - 1) / 2;
        break;
      default:
        return false;
    }
  }

  return m_ex->print(
    addressrange(m_ex, begin - 1, begin + count - 1),
    flags,
    separator);
}

bool wex::address::append(const std::string& text) const
{
  return add(add_t::APPEND, text);
}

bool wex::address::flags_supported(const std::string& flags)
{
  if (!flags.empty() && regex("([-+#pl]+)").match(flags) < 0)
  {
    log::status("Unsupported flags") << flags;
    return false;
  }

  return true;
}

int wex::address::get_line(int start_pos) const
{
  // We already have a line number, return that one.
  if (m_line >= 1 || m_ex == nullptr)
  {
    return m_line;
  }

  m_ex->get_stc()->set_search_flags(m_ex->search_flags());

  // Addressing in ex:
  // (5). A regular expression enclosed by <slash> characters ( '/' )
  // shall address the first line found by searching forward.
  // In addition, the second <slash> can be omitted at the end of a command
  // line.
  // Extra: if the extra slash is present, it might be preceded or followed
  // by an line number offset
  // (6). A regular expression enclosed in <question-mark> characters ( '?' )
  // shall address the first line found by searching backward.
  // In addition, the second <question-mark> can be omitted at the end of a
  // command line.
  // See also command-parser.cpp
  if (regex v(
        {"([-+]?[0-9]*)/(.*)/([-+]?[0-9]*)$",
         "/(.*)$",
         "([-+]?[0-9]*)\\?(.*)\\?([-+]?[0-9]*)$",
         "\\?(.*)$"});
      v.match(m_address) > 0)
  {
    int offset = 0, search_index = 0;

    switch (v.size())
    {
      case 3:
        if (!v[2].empty())
        {
          offset = stoi(v[2]);
        }
        else if (!v[0].empty())
        {
          offset = stoi(v[0]);
        }
        search_index = 1;
        break;

      case 2:
        if (!v[1].empty())
        {
          offset = stoi(v[1]);
        }
        break;
    }

    const auto use_pos =
      start_pos == -1 ? m_ex->get_stc()->GetCurrentPos() + 1 : start_pos;
    const auto& text(
      !v[search_index].empty() ? v[search_index] :
                                 find_replace_data::get()->get_find_string());
    const bool forward(v.match_no() == 0 || v.match_no() == 1);
    const auto result = offset + (!m_ex->get_stc()->is_visual() ?
                                    find_stream(m_ex, text, forward) :
                                    find_stc(m_ex, text, use_pos, forward));

    if (result > 0 && !m_ex->line_data().is_ctag())
    {
      find_replace_data::get()->set_find_string(text);
    }

    log::trace("address") << m_address << "gives" << result << "with offset"
                          << offset << "regex" << v.match_data().text() << "("
                          << v.match_no() << ")"
                          << "groups" << v.size();

    return result;
  }

  // Try address calculation. (1) (2) (3) (4)
  const auto& sum(m_ex->calculator(m_address));

  if (!sum)
  {
    return 0;
  }

  if (*sum < 0)
  {
    return 1;
  }

  if (*sum > m_ex->get_stc()->get_line_count())
  {
    return m_ex->get_stc()->get_line_count() == LINE_COUNT_UNKNOWN ?
             *sum :
             m_ex->get_stc()->get_line_count();
  }

  return *sum;
}

bool wex::address::insert(const std::string& text) const
{
  return add(add_t::INSERT, text);
}

bool wex::address::marker_add(char marker) const
{
  return get_line() > 0 && m_ex->marker_add(marker, get_line() - 1);
}

bool wex::address::marker_delete() const
{
  return m_address.size() > 1 && m_address[0] == '\'' &&
         m_ex->marker_delete(m_address[1]);
}

bool wex::address::parse(const command_parser& cp)
{
  m_address = cp.range();

  switch (cp.command()[0])
  {
    case 0:
      return false;

    case 'a':
      if (cp.text().contains('|'))
      {
        return append(find_after(cp.text(), "|"));
      }
      else
      {
        return m_ex->frame()->show_ex_input(m_ex->get_stc(), cp.command()[0]);
      }

    case 'i':
      if (cp.text().contains('|'))
      {
        return insert(find_after(cp.text(), "|"));
      }
      else
      {
        return m_ex->frame()->show_ex_input(m_ex->get_stc(), cp.command()[0]);
      }

    case 'k':
    case 'm':
      return !cp.text().empty() ? marker_add(cp.text()[0]) : false;

    case 'p':
      if (cp.command() == "pu")
      {
        return !cp.text().empty() ? put(cp.text()[0]) : put();
      }
      else
      {
        return false;
      }

    case 'r':
      return read(cp.text());

    case 'v':
      m_ex->get_stc()->visual(true);
      return true;

    case 'z':
      return adjust_window(cp.text());

    case '=':
      return write_line_number();

    default:
      log::status("Unknown address command") << cp.command();
      return false;
  }
}

bool wex::address::put(char name) const
{
  if (const auto line(get_line()); m_ex->get_stc()->GetReadOnly() ||
                                   m_ex->get_stc()->is_hexmode() || line <= 0)
  {
    return false;
  }

  m_ex->get_stc()->insert_text(
    m_ex->get_stc()->PositionFromLine(get_line()),
    m_ex->get_macros().get_register(name));
  return true;
}

bool wex::address::read(const std::string& arg) const
{
  if (
    m_ex->get_stc()->is_visual() &&
    (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode() ||
     get_line() <= 0))
  {
    return false;
  }

  if (arg.starts_with("!"))
  {
    factory::process process;

    if (process.system(arg.substr(1)) != 0)
    {
      return false;
    }

    return append(process.std_out());
  }

  path::current(m_ex->get_stc()->path().data().parent_path());

  file file(path(arg), std::ios_base::in);
  if (!file.is_open())
  {
    log::status(_("File")) << file.path() << "open error";
    return false;
  }

  if (const auto buffer(file.read()); buffer != nullptr)
  {
    if (!m_ex->get_stc()->is_visual())
    {
      m_ex->ex_stream()->insert_text(m_line, *buffer);
    }
    else if (m_address == ".")
    {
      m_ex->get_stc()->add_text(*buffer);
    }
    else
    {
      m_ex->get_stc()->insert_text(
        m_ex->get_stc()->PositionFromLine(get_line()),
        *buffer);
    }

    return true;
  }

  return false;
}

const std::string wex::address::regex_commands() const
{
  // Command Descriptions in ex.
  // 1addr commands
  return "(append\\b|a\\b|"
         "insert\\b|i\\b|"
         "mark\\b|ma\\b|k|"
         "pu\\b|"
         "read\\b|r\\b|"
         "visual\\b|vi\\b|"
         "z\\b|"
         "=)([\\s\\S]*)";
}

void wex::address::set_line(int line)
{
  if (line > m_ex->get_stc()->get_line_count())
  {
    m_line = m_ex->get_stc()->get_line_count();
  }
  else if (line < 1)
  {
    m_line = 1;
  }
  else
  {
    m_line = line;
  }
}

bool wex::address::write_line_number() const
{
  if (const auto line(get_line()); line <= 0)
  {
    return false;
  }

  m_ex->frame()->show_ex_message(std::to_string(get_line()));
  return true;
}
