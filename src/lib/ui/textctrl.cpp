////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.cpp
// Purpose:   Implementation of wex::textctrl class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex.h>
#include <wex/textctrl.h>

#include "textctrl-imp.h"

wex::textctrl::textctrl(
  managed_frame*     frame,
  wxControl*         prefix,
  const data::window& data)
  : m_imp(new textctrl_imp(this, prefix, data))
  , m_frame(frame)
{
}

wex::textctrl::textctrl(
  managed_frame*     frame,
  const std::string& value,
  const data::window& data)
  : m_imp(new textctrl_imp(this, value, data))
  , m_frame(frame)
{
}

wex::textctrl::~textctrl()
{
  delete m_imp;
}

wxControl* wex::textctrl::control()
{
  return m_imp;
}

const std::string wex::textctrl::get_text() const
{
  return m_imp->get_text();
}

void wex::textctrl::select_all() const
{
  return m_imp->SelectAll();
}

bool wex::textctrl::set_ex(wex::ex* ex, const std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  m_ex = ex;

  return m_imp->handle(command);
}

bool wex::textctrl::set_ex(wex::ex* ex, char command)
{
  m_ex = ex;
  return m_imp->handle(command);
}

void wex::textctrl::set_text(const std::string& text)
{
  m_imp->set_text(text);
}