/******************************************************************************\
* File:          config.cpp
* Purpose:       Implementation of wxExtension config class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/config.h>
#include <wx/extension/frd.h>

using namespace std;

wxExConfig::wxExConfig(
  const wxString& filename,
  long style)
#ifdef wxExUSE_PORTABLE
  : wxFileConfig(
#else
  : wxConfig(
#endif
      wxEmptyString,
      wxEmptyString,
      filename,
      wxEmptyString,
      style)
{
  m_FindReplaceData = new wxExFindReplaceData(this);
}

wxExConfig::~wxExConfig()
{
  delete m_FindReplaceData;
}
