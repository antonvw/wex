////////////////////////////////////////////////////////////////////////////////
// Name:      interruptable.cpp
// Purpose:   Implementation of class wex::interruptable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/interruptable.h>

bool wex::interruptable::m_Cancelled = false;
bool wex::interruptable::m_Running = false;

bool wex::interruptable::Cancel()
{
  if (!m_Running)
  {
    return false;  
  }
  
  m_Cancelled = true;
  m_Running = false;
  
  return true;
}

bool wex::interruptable::Start()
{
  if (m_Running)
  {
    return false;
  }

  m_Cancelled = false;
  m_Running = true;

  return true;
}

void wex::interruptable::Stop()
{
  m_Running = false;
}
