////////////////////////////////////////////////////////////////////////////////
// Name:      ex-commandline.cpp
// Purpose:   Implementation of wex::ex_commandline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ui/ex-commandline.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>

#include "ex-commandline-imp.h"

wex::ex_commandline::ex_commandline(
  wex::frame*         frame,
  wxControl*          prefix,
  const data::window& data)
  : m_imp(new ex_commandline_imp(this, prefix, data))
  , m_frame(frame)
{
}

wex::ex_commandline::ex_commandline(
  wex::frame*         frame,
  const std::string&  value,
  const data::window& data)
  : m_imp(new ex_commandline_imp(this, value, data))
  , m_frame(frame)
{
}

// there is no on_exit or destructor for deleting the m_imp
// when doing m_imp->Destroy(), the test-ex stream errors on exit

wex::syntax::stc* wex::ex_commandline::control()
{
  return m_imp;
}

bool wex::ex_commandline::find(bool user_input)
{
  if (m_stc == nullptr)
  {
    return false;
  }

  if (user_input)
  {
    m_stc->position_restore();
  }

  return m_stc->find(
    get_text(),
    m_imp->get_ex_command().str() == "@" ?
      find_replace_data::get()->stc_flags() :
      m_stc->vi_search_flags(),
    m_imp->get_ex_command().str() == "@" ?
      find_replace_data::get()->search_down() :
      m_imp->get_ex_command().str() == "/");
}

const std::string wex::ex_commandline::get_text() const
{
  return m_imp->get_text();
}

void wex::ex_commandline::on_key_down(wxKeyEvent& event)
{
  m_imp->on_key_down(event);
}

void wex::ex_commandline::select_all() const
{
  return m_imp->SelectAll();
}

void wex::ex_commandline::set_stc(wex::syntax::stc* stc)
{
  m_stc = stc;

  if (stc != nullptr)
  {
    m_imp->get_lexer().set(stc->get_lexer());
    m_imp->reset_margins();
  }
}

bool wex::ex_commandline::set_stc(
  wex::syntax::stc*  stc,
  const std::string& command)
{
  m_stc = stc;
  return m_imp->handle(command);
}

bool wex::ex_commandline::set_stc(wex::syntax::stc* stc, char command)
{
  m_stc = stc;
  return m_imp->handle(command);
}

void wex::ex_commandline::set_text(const std::string& text)
{
  m_imp->set_text(text);
}
