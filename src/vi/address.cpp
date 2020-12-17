////////////////////////////////////////////////////////////////////////////////
// Name:      address.cpp
// Purpose:   Implementation of class wex::address
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <memory>
#include <wex/address.h>
#include <wex/core.h>
#include <wex/ex.h>
#include <wex/log.h>
#include <wex/macros.h>
#include <wex/managed-frame.h>
#include <wex/process.h>
#include <wex/stc.h>

#define SEPARATE                                             \
  if (separator)                                             \
  {                                                          \
    output += std::string(40, '-') + m_ex->get_stc()->eol(); \
  }

wex::address::address(ex* ex, const std::string& address)
  : m_address(address)
  , m_ex(ex)
{
}

bool wex::address::adjust_window(const std::string& text) const
{
  std::vector<std::string> v;

  if (match("([-+=.^]*)([0-9]+)?(.*)", text, v) != 3)
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
    switch ((int)type.at(0))
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
  for (int i = begin; i < begin + count; i++)
  {
    char buffer[8];
    sprintf(buffer, "%6d ", i);

    output += (flags.find("#") != std::string::npos ? buffer : "") +
              m_ex->get_stc()->GetLine(i - 1);
  }
  SEPARATE;

  m_ex->frame()->print_ex(m_ex, output);

  return true;
}

bool wex::address::append(const std::string& text) const
{
  if (const auto line = get_line(); m_ex->get_stc()->GetReadOnly() ||
                                    m_ex->get_stc()->is_hexmode() || line <= 0)
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

bool wex::address::flags_supported(const std::string& flags) const
{
  if (flags.empty())
  {
    return true;
  }

  if (std::vector<std::string> v; match("([-+#pl])", flags, v) < 0)
  {
    log::status("Unsupported flags") << flags;
    return false;
  }

  return true;
}

int wex::address::get_line() const
{
  // We already have a line number, return that one.
  if (m_line >= 1)
  {
    return m_line;
  }

  m_ex->get_stc()->set_search_flags(m_ex->search_flags());

  // If this is a // address, return line with first forward match.
  if (std::vector<std::string> v; match("/(.*)/$", m_address, v) > 0)
  {
    m_ex->get_stc()->SetTargetRange(
      m_ex->get_stc()->GetCurrentPos(),
      m_ex->get_stc()->GetTextLength());

    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(
               m_ex->get_stc()->GetTargetStart()) +
             1;
    }

    m_ex->get_stc()->SetTargetRange(0, m_ex->get_stc()->GetCurrentPos());

    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(
               m_ex->get_stc()->GetTargetStart()) +
             1;
    }

    return 0;
  }
  // If this is a ?? address, return line with first backward match.
  else if (match("\\?(.*)\\?", m_address, v) > 0)
  {
    m_ex->get_stc()->SetTargetRange(m_ex->get_stc()->GetCurrentPos(), 0);

    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(
               m_ex->get_stc()->GetTargetStart()) +
             1;
    }

    m_ex->get_stc()->SetTargetRange(
      m_ex->get_stc()->GetTextLength(),
      m_ex->get_stc()->GetCurrentPos());

    if (m_ex->get_stc()->SearchInTarget(v[0]) != -1)
    {
      return m_ex->get_stc()->LineFromPosition(
               m_ex->get_stc()->GetTargetStart()) +
             1;
    }

    return 0;
  }

  // Try address calculation.
  if (const auto sum = m_ex->calculator(m_address); sum < 0)
  {
    return 1;
  }
  else if (sum > m_ex->get_stc()->get_line_count())
  {
    return m_ex->get_stc()->get_line_count();
  }
  else
  {
    return sum;
  }
}

bool wex::address::insert(const std::string& text) const
{
  if (const auto line = get_line(); m_ex->get_stc()->GetReadOnly() ||
                                    m_ex->get_stc()->is_hexmode() || line <= 0)
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

bool wex::address::parse(const std::string& command, const std::string& text)
  const
{
  switch (command[0])
  {
    case 0:
      return false;

    case 'a':
      if (text.find('|') != std::string::npos)
      {
        m_ex->frame()->show_ex_bar();
        return append(after(text, '|'));
      }
      else
      {
        return m_ex->frame()->show_ex_input(m_ex, command[0]);
      }

    case 'i':
      if (text.find('|') != std::string::npos)
      {
        m_ex->frame()->show_ex_bar();
        return insert(after(text, '|'));
      }
      else
      {
        return m_ex->frame()->show_ex_input(m_ex, command[0]);
      }

    case 'k':
      return !text.empty() ? marker_add(text[0]) : false;

    case 'p':
      if (command == "pu")
      {
        return !text.empty() ? put(text[0]) : put();
      }
      else
      {
        return false;
      }

    case 'r':
      return read(text);

    case 'v':
      m_ex->get_stc()->visual(true);
      return true;

    case 'z':
      return adjust_window(text);

    case '=':
      return write_line_number();

    default:
      log::status("Unknown address command") << command;
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
    m_ex->get_stc()->GetReadOnly() || m_ex->get_stc()->is_hexmode() ||
    get_line() <= 0)
  {
    return false;
  }

  if (arg.starts_with("!"))
  {
    process process;

    if (!process.execute(arg.substr(1), process::EXEC_WAIT))
    {
      return false;
    }

    return append(process.get_stdout());
  }
  else
  {
    path::current(m_ex->get_stc()->get_filename().get_path());

    if (file file(arg, std::ios_base::in); !file.is_open())
    {
      log::status(_("File")) << file.get_filename() << "open error";
      return false;
    }
    else if (const auto buffer(file.read()); buffer != nullptr)
    {
      if (m_address == ".")
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
  return std::string("(append\\b|"
                     "insert\\b|"
                     "mark\\b|ma\\b|"
                     "pu\\b|"
                     "read\\b|"
                     "visual\\b|vi\\b|"
                     "[aikrz=])([\\s\\S]*)");
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
