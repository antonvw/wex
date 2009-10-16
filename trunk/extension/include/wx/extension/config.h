/******************************************************************************\
* File:          config.h
* Purpose:       Declaration of wxExtension config class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXCONFIG_H
#define _EXCONFIG_H

/// Offers a general configuration.
#ifdef wxExUSE_PORTABLE
#include <wx/fileconf.h>
class wxExConfig : public wxFileConfig
#else
#include <wx/config.h>
class wxExConfig : public wxConfig
#endif
{
public:
  /// Default constructor.
  wxExConfig(
    const wxString& filename = wxEmptyString,
    long style = 0);
};
#endif
