/******************************************************************************\
* File:          statistics.cpp
* Purpose:       Implementation of statistics classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/statistics.h>

wxExFileStatistics::wxExFileStatistics()
{
}

long wxExFileStatistics::Get(const wxString& key) const
{
  std::map<wxString, long>::const_iterator it = m_Elements.GetItems().find(key);

  if (it != m_Elements.GetItems().end())
  {
    return it->second;
  }
  else
  {
    std::map<wxString, long>::const_iterator it = m_Keywords.GetItems().find(key);

    if (it != m_Keywords.GetItems().end())
    {
      return it->second;
    }
  }

  return 0;
}
