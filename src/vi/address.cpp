////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/file.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/factory/process.h>
#include <wex/factory/stc.h>
#include <wex/ui/frame.h>
#include <wex/vi/address.h>
#include <wex/vi/command-parser.h>
#include <wex/vi/ex-stream.h>
#include <wex/vi/ex.h>
#include <wex/vi/macros.h>

#include "util.h"

#define SEARCH_TARGET                                                         \
  if (ex->get_stc()->SearchInTarget(text) != -1)                              \
  {                                                                           \
    return ex->get_stc()->LineFromPosition(ex->get_stc()->GetTargetStart()) + \
           1;                                                                 \
  }

#define SEPARATE                                             \
  if (separator)                                             \
  {                                                          \
    output += std::string(40, '-') + m_ex->get_stc()->eol(); \
  }

namespace wex
{
int find_stc(ex* ex, const std::string& text, int start_pos, bool forward)
{
  if (forward)
  {
    ex->get_stc()->SetTargetRange(start_pos, ex->get_stc()->GetTextLength());
  }
  else
  {
    ex->get_stc()->SetTargetRange(start_pos, 0);
  }

  SEARCH_TARGET;

  if (forward)
  {
    ex->get_stc()->SetTargetRange(0, start_pos);
  }
  else
  {
    ex->get_stc()->SetTargetRange(ex->get_stc()->GetTextLength(), start_pos);
  }

  SEARCH_TARGET;

  return 0;
}

int find_stream(ex* ex, const std::string& text, bool forward)
{
  if (ex->ex_stream()->find(text, -1, forward))
  {
    return ex->ex_stream()->get_current_line() + 1;
  }
  else
  {
    return 0;
  }
}
}; // namespace wex

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

bool wex::address::adjust_window(const std::string& text) const
{
  regex v("([-+=.^]*)([0-9]+)?(.*)");

  if (v.match(text) != 3)
  {
    return false;
  }

  const auto  count = (v[1].empty() ? 2 : std::stoi(v[1]));
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

  std::string output;

  SEPARATE;
  output +=
    wex::get_lines(m_ex->get_stc(), begin - 1, begin + count - 1, flags);
  SEPARATE;

  m_ex->print(output);

  return true;
}

bool wex::address::append(const std::string& text) const
{
  if (const auto line = get_line(); line <= 0)
  {
    return false;
  }
  else if (!m_ex->get_stc()->is_visual())
  {
    m_ex->ex_stream()->insert_text(*this, text, ex_stream::INSERT_AFTER);
    return true;
  }
  else if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode())
  {
    return false;
  }
  else
  {
    m_ex->get_stc()->insert_text(m_ex->get_stc()->PositionFromLine(line), text);
    m_ex->get_stc()->goto_line(line + get_number_of_lines(text) - 1);
    return true;
  }
}

bool wex::address::flags_supported(const std::string& flags)
{
  if (flags.empty())
  {
    return true;
  }

  if (regex("([-+#pl]+)").match(flags) < 0)
  {
    log::status("Unsupported flags") << flags;
    return false;
  }

  return true;
}

int wex::address::get_line(int start_pos) const
{
  // We already have a line number, return that one.
  if (m_line >= 1)
  {
    return m_line;
  }

  m_ex->get_stc()->set_search_flags(m_ex->search_flags());

  // If this is a //, ?? address, return line with first forward, backward
  // match.
  if (regex v({std::string("/(.*)/$"), "\\?(.*)\\?$"}); v.match(m_address) > 0)
  {
    const auto use_pos =
      start_pos == -1 ? m_ex->get_stc()->GetCurrentPos() : start_pos;

    return !m_ex->get_stc()->is_visual() ?
             find_stream(m_ex, v[0], m_address[0] == '/') :
             find_stc(m_ex, v[0], use_pos, m_address[0] == '/');
  }
  // Try address calculation.
  else if (const auto sum = m_ex->calculator(m_address); sum < 0)
  {
    return 1;
  }
  else if (sum > m_ex->get_stc()->get_line_count())
  {
    return m_ex->get_stc()->get_line_count() == LINE_COUNT_UNKNOWN ?
             sum :
             m_ex->get_stc()->get_line_count();
  }
  else
  {
    return sum;
  }
}

bool wex::address::insert(const std::string& text) const
{
  if (const auto line = get_line(); line <= 0)
  {
    return false;
  }
  else if (!m_ex->get_stc()->is_visual())
  {
    m_ex->ex_stream()->insert_text(*this, text);
    return true;
  }
  else if (m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode())
  {
    return false;
  }
  else
  {
    m_ex->get_stc()->insert_text(
      m_ex->get_stc()->PositionFromLine(line - 1),
      text);
    return true;
  }
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
      if (cp.text().find('|') != std::string::npos)
      {
        return append(find_after(cp.text(), "|"));
      }
      else
      {
        return m_ex->frame()->show_ex_input(m_ex->get_stc(), cp.command()[0]);
      }

    case 'i':
      if (cp.text().find('|') != std::string::npos)
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
  if (const auto line = get_line(); m_ex->get_stc()->GetReadOnly() ||
                                    m_ex->get_stc()->is_hexmode() || line <= 0)
  {
    return false;
  }
  else
  {
    m_ex->get_stc()->insert_text(
      m_ex->get_stc()->PositionFromLine(get_line()),
      m_ex->get_macros().get_register(name));
    return true;
  }
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
  else
  {
    path::current(m_ex->get_stc()->path().data().parent_path());

    if (file file(path(arg), std::ios_base::in); !file.is_open())
    {
      log::status(_("File")) << file.path() << "open error";
      return false;
    }
    else if (const auto buffer(file.read()); buffer != nullptr)
    {
      if (!m_ex->get_stc()->is_visual())
      {
        m_ex->ex_stream()->insert_text(*this, *buffer);
      }
      else if (m_address == ".")
      {
        m_ex->get_stc()->add_text(*buffer);
      }
      else
      {
        m_ex->get_stc()->insert_text(
          m_ex->get_stc()->PositionFromLine(get_line()),
          buffer->data());
      }
      return true;
    }
    else
    {
      return false;
    }
  }
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
  if (const auto line = get_line(); line <= 0)
  {
    return false;
  }
  else
  {
    m_ex->frame()->show_ex_message(std::to_string(get_line()));
    return true;
  }
}
