////////////////////////////////////////////////////////////////////////////////
// Name:      control-data.cpp
// Purpose:   Implementation of wex::control_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/control-data.h>

wex::control_data& wex::control_data::col(int col)
{
  m_Col = col;
  return *this;
}
  
wex::control_data& wex::control_data::command(const std::string& command)
{
  m_Command = command;
  return *this;
}
  
wex::control_data& wex::control_data::find(
  const std::string& text,
  int flags) 
{
  m_Find = text;
  m_find_flags = flags;
  
  return *this;
}

bool wex::control_data::inject(
  std::function<bool(void)> line,
  std::function<bool(void)> col,
  std::function<bool(void)> find,
  std::function<bool(void)> command) const
{
  bool injected = false;
  
  if (m_Line != DATA_NUMBER_NOT_SET && line != nullptr)
  {
    if (line())
    {
      injected = true;
    }
  }
  
  if (m_Col != DATA_NUMBER_NOT_SET && col != nullptr)
  {
    if (col())
    {
      injected = true;
    }
  }
  
  if (!m_Find.empty() && find != nullptr)
  {
    if (find())
    {
      injected = true;
    }
  }

  if (!m_Command.empty() && command != nullptr)
  {
    if (command())
    {
      injected = true;
    }
  }

  return injected;
}

wex::control_data& wex::control_data::line(int line, std::function<int(int)> valid)
{
  m_Line = (valid != nullptr ? valid(line): line);

  return *this;
}

void wex::control_data::reset()
{
  m_Col = DATA_NUMBER_NOT_SET;
  m_Command.clear();
  m_Find.clear();
  m_Line = DATA_NUMBER_NOT_SET;
  m_is_required = false;
  m_Validator = nullptr;
}

wex::control_data& wex::control_data::validator(wxValidator* validator)
{
  m_Validator = validator;

  return *this;
}
