////////////////////////////////////////////////////////////////////////////////
// Name:      data/control.cpp
// Purpose:   Implementation of wex::control_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/control-data.h>

wex::control_data& wex::control_data::col(int col)
{
  m_col = col;
  return *this;
}

wex::control_data& wex::control_data::command(const std::string& command)
{
  if (!command.empty())
  {
    m_command.set(command);
  }

  return *this;
}

wex::control_data& wex::control_data::find(const std::string& text, int flags)
{
  m_find       = text;
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

  if (m_line != DATA_NUMBER_NOT_SET && line != nullptr)
  {
    if (line())
    {
      injected = true;
    }
  }

  if (m_col != DATA_NUMBER_NOT_SET && col != nullptr)
  {
    if (col())
    {
      injected = true;
    }
  }

  if (!m_find.empty() && find != nullptr)
  {
    if (find())
    {
      injected = true;
    }
  }

  if (!m_command.empty() && command != nullptr)
  {
    if (command())
    {
      injected = true;
    }
  }

  return injected;
}

wex::control_data&
wex::control_data::line(int line, std::function<int(int)> valid)
{
  m_line = (valid != nullptr ? valid(line) : line);

  return *this;
}

void wex::control_data::reset()
{
  m_col = DATA_NUMBER_NOT_SET;
  m_command.clear();
  m_find.clear();
  m_line        = DATA_NUMBER_NOT_SET;
  m_is_required = false;
  m_validator   = nullptr;
}

wex::control_data& wex::control_data::validator(wxValidator* validator)
{
  m_validator = validator;

  return *this;
}
