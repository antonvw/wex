////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.cpp
// Purpose:   Implementation of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "interruptible-imp.h"

void wex::interruptible_imp::cancel()
{
  m_cancelled.store(true);
  m_running.store(false);
}

void wex::interruptible_imp::start()
{
  m_cancelled.store(false);
  m_running.store(true);
}

void wex::interruptible_imp::stop()
{
  m_cancelled.store(false);
  m_running.store(false);
}
