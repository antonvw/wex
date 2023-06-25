////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.cpp
// Purpose:   Implementation of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/interruptible.h>

bool wex::interruptible::end()
{
  if (!m_running)
  {
    return false;
  }

  m_running.store(false);

  return true;
}

bool wex::interruptible::is_running()
{
  return m_running;
}

bool wex::interruptible::start()
{
  if (m_running)
  {
    return false;
  }

  m_running.store(true);

  return true;
}
