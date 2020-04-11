////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.cpp
// Purpose:   Implementation of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-command.h>
#include <wex/stc.h>
#include <wex/vi.h>

wex::ex_command::ex_command() {}

wex::ex_command::ex_command(const std::string& command)
  : m_text(command)
{
}

wex::ex_command::ex_command(wex::stc* stc)
  : m_stc(stc)
  , m_stc_original(stc)
{
}

wex::ex_command::ex_command(const ex_command& c)
{
  *this = c;
}

wex::ex_command& wex::ex_command::operator=(const ex_command& c)
{
  if (this != &c)
  {
    m_text = c.m_text;

    if (c.m_stc != nullptr)
    {
      m_stc = c.m_stc;
    }

    if (c.m_stc_original != nullptr)
    {
      m_stc_original = c.m_stc_original;
    }
  }

  return *this;
}

bool wex::ex_command::append_exec(char c)
{
  append(c);
  return exec();
}

bool wex::ex_command::exec() const
{
  return m_stc != nullptr && m_stc->get_vi().command(command());
}

wex::ex_command& wex::ex_command::reset(const std::string& text)
{
  m_text = m_text.substr(0, str().size()) + text;
  return *this;
}
  
void wex::ex_command::restore(const ex_command& c)
{
  m_text = c.m_text;

  if (c.m_stc != nullptr || c.m_stc_original != nullptr)
  {
    m_stc = (c.m_stc_original != nullptr ? c.m_stc_original : c.m_stc);
  }
}

wex::ex_command& wex::ex_command::set(const std::string& text)
{
  assert(!text.empty());
  m_text = text;
  return *this;
}

void wex::ex_command::set(const ex_command& c)
{
  m_text = c.m_text;

  if (c.m_stc != nullptr || c.m_stc_original != nullptr)
  {
    m_stc = (c.m_stc == c.m_stc_original ? c.m_stc : c.m_stc_original);
  }
}

std::string wex::ex_command::str() const
{
  switch (type())
  {
    case type_t::NONE:
      return std::string();

    case type_t::CALC:
      return m_text.substr(0, 2);

    default:
      return m_text.substr(0, 1);
  }
}

wex::ex_command::type_t wex::ex_command::type() const
{
  if (m_text.empty())
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
        return type_t::COMMAND;

      case '!':
        return type_t::EXEC;

      case '/':
      case '?':
        return m_stc != nullptr && m_stc->get_margin_text_click() > 0 ?
                 type_t::FIND_MARGIN :
                 type_t::FIND;

      default:
        return type_t::VI;
    }
}
