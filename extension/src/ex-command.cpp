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
    m_IsHandled = c.m_IsHandled;

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
  m_IsHandled = false;
  return 0;
}

bool wex::ex_command::AppendExec(char c)
{
  Append(c);
  return Exec();
}

bool wex::ex_command::Exec(const std::string& command)
{
  return m_IsHandled || ((m_STC != nullptr) && (command.empty() ? 
    m_STC->GetVi().Command(Command()):
    m_STC->GetVi().Command(command)));
}

void wex::ex_command::Restore(const ex_command& c)
{
  m_Command = c.m_Command;
  m_IsHandled = c.m_IsHandled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC_original != nullptr ? c.m_STC_original: c.m_STC);
  }
}

void wex::ex_command::Set(const ex_command& c)
{
  m_Command = c.m_Command;
  m_IsHandled = c.m_IsHandled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC == c.m_STC_original ? c.m_STC: c.m_STC_original);
  }
}

wex::ex_command::type wex::ex_command::Type() const
{
  if (m_Command.empty()) 
  {
    return type::NONE;
  }
  else switch (m_Command[0])
  {
    case ':': return type::COMMAND;
    case '=': return type::CALC;
    case '!': return type::EXEC;
    
    case '/':
    case '?': 
      return m_STC != nullptr && m_STC->GetMarginTextClick() > 0 ?
        type::FIND_MARGIN: type::FIND;
    
    default: return type::VI;
  }
}
