////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Include file for wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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

    /// Constructs the config, initializes the locale, loads the VCS file.
    /// In your own OnInit first set the app name,
    /// as it uses this name for the config,
    /// and then call this base class method.
    bool OnInit() override;

    /// Deletes all global objects and cleans up things if necessary.
    /// You should normally don't need to override it.
    int OnExit() override;

    /// Other methods

    /// Returns the catalog dir.
    const auto& get_catalog_dir() const { return m_catalog_dir; }

    /// Returns the locale.
    const auto& get_locale() const { return m_locale; }

  private:
    std::string m_catalog_dir;
    wxLocale    m_locale;
  };
}; // namespace wex
