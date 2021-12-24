////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.cpp
// Purpose:   Implementation of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/interruptible.h>

#include "interruptible-imp.h"

bool wex::interruptible::cancel()
{
  if (!m_imp->is_running())
  {
    return false;
  }

  m_imp->cancel();

  return true;
}

void wex::interruptible::on_exit()
{
  delete m_imp;
}

void wex::interruptible::on_init()
{
  m_imp = new interruptible_imp;
}

bool wex::interruptible::is_cancelled()
{
  return m_imp->is_cancelled();
}

bool wex::interruptible::is_running()
{
  return m_imp->is_running();
}

bool wex::interruptible::start()
{
  if (is_running())
  {
    return false;
  }

  m_imp->start();

  return true;
}

void wex::interruptible::stop()
{
  m_imp->stop();
}
