/******************************************************************************\
* File:          app.h
* Purpose:       Include file for wxExApp class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXAPP_H
#define _EXAPP_H

#include <wx/app.h>
#include <wx/intl.h> // for wxLocale

/// Offers the application, with wxExtension specific init and exit and keeps the locale.
/// Your application should be derived from this class.
class wxExApp : public wxApp
{
public:
  /// Gets the locale.
  const wxLocale& GetLocale() const {return m_Locale;};

  /// Constructs the config, lexers and printer (and reads the lexers).
  /// Initializes the locale and wxExTool.
  /// In your class first set the app name, as it uses this name for the config file.
  /// See for documentation the lexers.xml file.
  virtual bool OnInit();

  /// This destroys (and so writes) the config, lexers, printer
  /// and cleans up things if necessary.
  virtual int OnExit();
private:
  wxLocale m_Locale;
};
#endif
