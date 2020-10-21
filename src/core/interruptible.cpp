////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.cpp
// Purpose:   Implementation of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/interruptible.h>

bool wex::interruptible::cancel()
{
  if (!m_running)
  {
    return false;
  }

  m_cancelled = true;
  m_running   = false;

  return true;
}

bool wex::interruptible::start()
{
  if (m_running)
  {
    return false;
  }

  m_cancelled = false;
  m_running   = true;

  return true;
}

void wex::interruptible::stop()
{
  m_running = false;
}
