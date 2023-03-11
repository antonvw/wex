////////////////////////////////////////////////////////////////////////////////
// Name:      app-locale.h
// Purpose:   Declaration of wex::file_translations_loader class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/translation.h>

namespace wex
{
/// This class offers translations.
class file_translations_loader : public wxFileTranslationsLoader
{
public:
  /// Returns the catalog dir found.
  static auto catalog_dir() { return m_catalog_dir; }

  /// Adds catalogs.
  void add_catalogs(wxLanguage language);

  /// Loads catalog.
  wxMsgCatalog*
  LoadCatalog(const wxString& domain, const wxString& lang) override;

private:
  static inline std::string m_catalog_dir;
  std::string               m_catalog_file;
};
}; // namespace wex
