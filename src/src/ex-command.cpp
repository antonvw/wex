////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.cpp
// Purpose:   Implementation of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex-command.h>
#include <wex/stc.h>
#include <wex/vi.h>

wex::ex_command::ex_command(const std::string& command)
  : m_Command(command)
{
}

wex::ex_command::ex_command(wex::stc* stc)
  : m_STC(stc) 
  , m_STC_original(stc)
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
    m_Command = c.m_Command;
    m_is_handled = c.m_is_handled;

    if (c.m_STC != nullptr)
    {
      m_STC = c.m_STC;
    }

    if (c.m_STC_original != nullptr)
    {
      m_STC_original = c.m_STC_original;
    }
  }

  return *this;
}

size_t wex::ex_command::clear()
{
  m_Command.clear(); 
  m_is_handled = false;
  return 0;
}

bool wex::ex_command::append_exec(char c)
{
  append(c);
  return exec();
}

bool wex::ex_command::exec(const std::string& cmd)
{
  return m_is_handled || ((m_STC != nullptr) && (cmd.empty() ? 
    m_STC->get_vi().command(command()):
    m_STC->get_vi().command(cmd)));
}

void wex::ex_command::restore(const ex_command& c)
{
  m_Command = c.m_Command;
  m_is_handled = c.m_is_handled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC_original != nullptr ? c.m_STC_original: c.m_STC);
  }
}

void wex::ex_command::set(const ex_command& c)
{
  m_Command = c.m_Command;
  m_is_handled = c.m_is_handled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC == c.m_STC_original ? c.m_STC: c.m_STC_original);
  }
}

wex::ex_command::type_t wex::ex_command::type() const
{
  if (m_Command.empty()) 
  {
    return type_t::NONE;
  }
  else switch (m_Command[0])
  {
    case ':': return type_t::COMMAND;
    case '=': return type_t::CALC;
    case '!': return type_t::EXEC;
    
    case '/':
    case '?': 
      return m_STC != nullptr && m_STC->get_margin_text_click() > 0 ?
        type_t::FIND_MARGIN: type_t::FIND;
    
    default: return type_t::VI;
  }
}
