////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.cpp
// Purpose:   Implementation of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/ex-command.h>
#include <wex/factory/line-data.h>
#include <wex/factory/stc.h>

wex::ex_command::ex_command(const std::string& command)
  : m_text(command)
{
}

wex::ex_command::ex_command(wex::factory::stc* stc)
  : m_stc(stc)
{
}

bool wex::ex_command::append_exec(char c)
{
  append(c);
  return exec();
}

bool wex::ex_command::exec() const
{
  return m_stc != nullptr && m_stc->vi_command(line_data().command(m_text));
}

bool wex::ex_command::exec_finish(bool user_input) const
{
  return m_stc != nullptr && m_stc->vi_command_finish(user_input);
}

void wex::ex_command::no_type()
{
  m_has_type = false;
}

wex::ex_command& wex::ex_command::reset(const std::string& text)
{
  m_text = m_has_type ? m_text.substr(0, str().size()) + text : text;

  return *this;
}

const std::string wex::ex_command::selection_range()
{
  return "'<,'>";
}

wex::ex_command& wex::ex_command::set(const std::string& text)
{
  m_text = text;

  return *this;
}

std::string wex::ex_command::str() const
{
  if (!m_has_type)
  {
    return m_text;
  }
  else
  {
    switch (type())
    {
      case type_t::NONE:
        return std::string();

      case type_t::CALC:
        return m_text.substr(0, 2);

      // : + selection_range
      case type_t::COMMAND_RANGE:
        return m_text.substr(0, selection_range().size() + 1);

      // : + selection_range + !
      case type_t::ESCAPE_RANGE:
        return m_text.substr(0, selection_range().size() + 2);

      default:
        return m_text.substr(0, 1);
    }
  }
}

wex::ex_command::type_t wex::ex_command::type() const
{
  if (m_text.empty() || !m_has_type)
  {
    return type_t::NONE;
  }
  else
    switch (m_text[0])
    {
      case WXK_CONTROL_R:
        return m_text.size() > 1 && m_text[1] == '=' ? type_t::CALC :
                                                       type_t::NONE;

      case ':':
        if (m_stc != nullptr && !m_stc->is_visual())
        {
          return type_t::COMMAND_EX;
        }
        else if (m_text.starts_with(":" + selection_range() + "!"))
        {
          return type_t::ESCAPE_RANGE;
        }
        else if (m_text.starts_with(":" + selection_range()))
        {
          return type_t::COMMAND_RANGE;
        }
        else
        {
          return type_t::COMMAND;
        }

      case '!':
        return type_t::ESCAPE;

      case '/':
      case '?':
        return m_stc != nullptr && m_stc->get_margin_text_click() > 0 ?
                 type_t::FIND_MARGIN :
                 type_t::FIND;

      default:
        return type_t::VI;
    }
}
