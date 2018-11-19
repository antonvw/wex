////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Include file for wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>
#include <wx/intl.h> // for wxLocale

namespace wex
{
  /// Offers the application, with lib specific init and exit.
  /// It also keeps the locale and the catalog dir.
  /// Your application should be derived from this class.
  class app : public wxApp
  {
  public:
    /// Virtual interface

    /// This function is called when an assert failure occurs, 
    /// i.e. the condition specified in assert() macro evaluated to false.
    virtual void OnAssertFailure(
      const wxChar *file, int line, const wxChar *func, 
      const wxChar *cond, const wxChar *msg) override;
    
    /// Constructs the config, initializes the locale, loads the VCS file.
    /// In your own OnInit first set the app name,
    /// as it uses this name for the config,
    /// and then call this base class method.
    virtual bool OnInit() override;

    /// Deletes all global objects and cleans up things if necessary.
    /// You should normally don't need to override it.
    virtual int OnExit() override;
    
    /// Other methods
    
    /// Returns the catalog dir.
    const auto & get_catalog_dir() const {return m_CatalogDir;};
    
    /// Returns the locale.
    const auto & get_locale() const {return m_Locale;};
  private:
    std::string m_CatalogDir;
    wxLocale m_Locale;
  };
};
