////////////////////////////////////////////////////////////////////////////////
// Name:      interruptible.cpp
// Purpose:   Implementation of class wex::interruptible
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "interruptible-imp.h"

void wex::interruptible_imp::end()
{
  m_running.store(false);
}

void wex::interruptible_imp::start()
{
  m_running.store(true);
}
