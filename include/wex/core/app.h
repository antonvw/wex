////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Include file for wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/app.h>
#include <wx/uilocale.h>

namespace wex
{
class file_translations_loader;

/// Offers the application, with lib specific init and exit,
/// and provides access to the locale and the catalog dir.
/// Your application should be derived from this class.
class app : public wxApp
{
public:
  // Static interface.

  /// Returns the locale.
  static const wxUILocale& get_locale();

  // Virtual interface

  /// Constructs the config, initializes the locale, loads the VCS file.
  /// In your own OnInit first set the app name,
  /// as it uses this name for the config,
  /// and then call this base class method.
  bool OnInit() override;

  /// Deletes all global objects and cleans up things if necessary.
  /// You should normally don't need to override it.
  int OnExit() override;

  // Other methods

  /// Returns the catalog dir.
  const std::string get_catalog_dir() const;

  /// Returns locale language.
  auto get_language() const { return m_language; }

protected:
  virtual wxLanguage get_default_language() const;

private:
  void set_language();

  wxLanguage m_language;

  file_translations_loader* m_loader{nullptr};

  static int m_first_init;
};
}; // namespace wex
