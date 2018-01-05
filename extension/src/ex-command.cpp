////////////////////////////////////////////////////////////////////////////////
// Name:      ex-command.cpp
// Purpose:   Implementation of class wxExExCommand
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/ex-command.h>
#include <wx/extension/stc.h>
#include <wx/extension/vi.h>

wxExExCommand::wxExExCommand(const std::string command)
  : m_Command(command)
{
}

wxExExCommand::wxExExCommand(wxExSTC* stc)
  : m_STC(stc) 
  , m_STC_original(stc)
{
}

wxExExCommand::wxExExCommand(const wxExExCommand& c)
{
  *this = c;
}
  
wxExExCommand& wxExExCommand::operator=(const wxExExCommand& c)
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

size_t wxExExCommand::clear()
{
  m_Command.clear(); 
  m_IsHandled = false;
  return 0;
}

bool wxExExCommand::AppendExec(char c)
{
  Append(c);
  return Exec();
}

bool wxExExCommand::Exec(const std::string& command)
{
  if (m_STC == nullptr) return false;

  return command.empty() ? 
    m_STC->GetVi().Command(Command()):
    m_STC->GetVi().Command(command);
}

void wxExExCommand::Restore(const wxExExCommand& c)
{
  m_Command = c.m_Command;
  m_IsHandled = c.m_IsHandled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC_original != nullptr ? c.m_STC_original: c.m_STC);
  }
}

void wxExExCommand::Set(const wxExExCommand& c)
{
  m_Command = c.m_Command;
  m_IsHandled = c.m_IsHandled;

  if (c.m_STC != nullptr || c.m_STC_original != nullptr)
  {
    m_STC = (c.m_STC == c.m_STC_original ? c.m_STC: c.m_STC_original);
  }
}
