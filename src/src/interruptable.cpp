////////////////////////////////////////////////////////////////////////////////
// Name:      interruptable.cpp
// Purpose:   Implementation of class wex::interruptable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/interruptable.h>

bool wex::interruptable::m_cancelled = false;
bool wex::interruptable::m_running = false;

bool wex::interruptable::cancel()
{
  if (!m_running)
  {
    return false;  
  }
  
  m_cancelled = true;
  m_running = false;
  
  return true;
}

bool wex::interruptable::start()
{
  if (m_running)
  {
    return false;
  }

  m_cancelled = false;
  m_running = true;

  return true;
}

void wex::interruptable::stop()
{
  m_running = false;
}
