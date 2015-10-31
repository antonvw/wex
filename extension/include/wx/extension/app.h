////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Include file for wxExApp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>
#include <wx/intl.h> // for wxLocale

/// Offers the application, with wxExtension specific init and exit.
/// It also keeps the locale and the catalog dir.
/// Your application should be derived from this class.
class WXDLLIMPEXP_BASE wxExApp : public wxApp
{
public:
  /// Returns the catalog dir.
  const wxString& GetCatalogDir() const {return m_CatalogDir;};
  
  /// Returns the locale.
  const auto & GetLocale() const {return m_Locale;};

  /// Constructs the config, initializes the locale, loads the VCS file.
  /// In your own OnInit first set the app name,
  /// as it uses this name for the config,
  /// and then call this base class method.
  virtual bool OnInit();

  /// Deletes all global objects and cleans up things if necessary.
  /// You should normally don't need to override it.
  virtual int OnExit();
private:
  wxString m_CatalogDir;
  wxLocale m_Locale;
};
