////////////////////////////////////////////////////////////////////////////////
// Name:      textctrl.cpp
// Purpose:   Implementation of wex::textctrl class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/frame.h>
#include <wex/ui/textctrl.h>

#include "textctrl-imp.h"

wex::textctrl::textctrl(
  wex::frame*         frame,
  wxControl*          prefix,
  const data::window& data)
  : m_imp(new textctrl_imp(this, prefix, data))
  , m_frame(frame)
{
}

wex::textctrl::textctrl(
  wex::frame*         frame,
  const std::string&  value,
  const data::window& data)
  : m_imp(new textctrl_imp(this, value, data))
  , m_frame(frame)
{
}

wxControl* wex::textctrl::control()
{
  return m_imp;
}

const std::string wex::textctrl::get_text() const
{
  return m_imp->get_text();
}

void wex::textctrl::on_exit()
{
  m_imp->Destroy();
}

void wex::textctrl::select_all() const
{
  return m_imp->SelectAll();
}

bool wex::textctrl::set_stc(wex::factory::stc* stc, const std::string& command)
{
  set_stc(stc);
  return m_imp->handle(command);
}

bool wex::textctrl::set_stc(wex::factory::stc* stc, char command)
{
  set_stc(stc);
  return m_imp->handle(command);
}

void wex::textctrl::set_text(const std::string& text)
{
  m_imp->set_text(text);
}
