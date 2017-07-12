////////////////////////////////////////////////////////////////////////////////
// Name:      interruptable.cpp
// Purpose:   Implementation of class wxExInterruptable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/interruptable.h>

bool wxExInterruptable::m_Cancelled = false;
bool wxExInterruptable::m_Running = false;

bool wxExInterruptable::Cancel()
{
  if (!m_Running)
  {
    return false;  
  }
  
  m_Cancelled = true;
  m_Running = false;
  
  return true;
}

bool wxExInterruptable::Start()
{
  if (m_Running)
  {
    return false;
  }

  m_Cancelled = false;
  m_Running = true;

  return true;
}

void wxExInterruptable::Stop()
{
  m_Running = false;
}
