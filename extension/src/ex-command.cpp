////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.cpp
// Purpose:   Implementation of class wex::ex_command
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/ex-command.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>

wex::ex_command::ex_command(const std::string& command)
  : m_Command(command)
{
}

wex::ex_command::ex_command(wex::stc* stc)
  : m_STC(stc) 
  , m_STC_original(stc)
{
}

wex::ex_command::ex_command(const wex::ex_command& c)
{
  *this = c;
}
  
wex::ex_command& wex::ex_command::operator=(const wex::ex_command& c)
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

void wex::ex_command::Restore(const wex::ex_command& c)
{
  m_Command = c.m_Command;
  m_IsHandled = c.m_IsHandled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC_original != nullptr ? c.m_STC_original: c.m_STC);
  }
}

void wex::ex_command::Set(const wex::ex_command& c)
{
  m_Command = c.m_Command;
  m_IsHandled = c.m_IsHandled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC == c.m_STC_original ? c.m_STC: c.m_STC_original);
  }
}

wex::ex_command_type wex::ex_command::Type() const
{
  if (m_Command.empty()) 
  {
    return wex::ex_command_type::NONE;
  }
  else switch (m_Command[0])
  {
    case ':': return wex::ex_command_type::COMMAND;
    case '=': return wex::ex_command_type::CALC;
    case '/':
    case '?': 
      return m_STC != nullptr && m_STC->GetMarginTextClick() > 0 ?
        wex::ex_command_type::FIND_MARGIN:
        wex::ex_command_type::FIND;
    case '!': return wex::ex_command_type::EXEC;
    default: return wex::ex_command_type::VI;
  }
}
