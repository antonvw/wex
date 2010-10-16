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

/// Offers the application, with wxExtension specific init and exit.
/// It also keeps the locale and the catalog dir.
/// Your application should be derived from this class.
class WXDLLIMPEXP_BASE wxExApp : public wxApp
{
public:
  /// Gets the catalog dir.
  const wxString& GetCatalogDir() const {return m_CatalogDir;};
  
  /// Gets the locale.
  const wxLocale& GetLocale() const {return m_Locale;};

  /// Constructs the config, and initializes the locale.
  /// In your class OnInit first set the app name,
  /// as it uses this name for the config,
  /// and then call this base class method.
  virtual bool OnInit();

  /// This deletes all global objects and cleans up things if necessary.
  /// You should normally don't need to override it.
  virtual int OnExit();
private:
  wxString m_CatalogDir;
  wxLocale m_Locale;
};
#endif
