////////////////////////////////////////////////////////////////////////////////
// Name:      data/control.cpp
// Purpose:   Implementation of wex::data::control
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/control.h>

wex::data::control& wex::data::control::find(const std::string& text, int flags)
{
  m_find       = text;
  m_find_flags = flags;

  return *this;
}

bool wex::data::control::inject(
  std::function<bool(void)> f_line,
  std::function<bool(void)> f_col,
  std::function<bool(void)> f_find,
  std::function<bool(void)> f_command) const
{
  bool injected = false;

  if (line_data::line() != NUMBER_NOT_SET && f_line != nullptr)
  {
    if (f_line())
    {
      injected = true;
    }
  }

  if (col() != NUMBER_NOT_SET && f_col != nullptr)
  {
    if (f_col())
    {
      injected = true;
    }
  }

  if (!m_find.empty() && f_find != nullptr)
  {
    if (f_find())
    {
      injected = true;
    }
  }

  if (!command().empty() && f_command != nullptr)
  {
    if (f_command())
    {
      injected = true;
    }
  }

  return injected;
}

void wex::data::control::reset()
{
  m_find.clear();
  m_is_required = false;
  m_validator   = nullptr;

  wex::line_data::reset();
}

wex::data::control& wex::data::control::validator(wxValidator* validator)
{
  m_validator = validator;

  return *this;
}
