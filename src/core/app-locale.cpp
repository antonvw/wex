////////////////////////////////////////////////////////////////////////////////
// Name:      app-locale.cpp
// Purpose:   Implementation of wex::file_translations_loader class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wx/stdpaths.h>
#include <wx/uilocale.h>

#include <filesystem>
#include <numeric>

#include "app-locale.h"

namespace fs = std::filesystem;

void wex::file_translations_loader::add_catalogs(wxLanguage language)
{
  std::vector<std::string> dirs{
    wxStandardPaths::Get().GetExecutablePath() + ".app/Contents/Resources/" +
      wxUILocale::GetLanguageCanonicalName(language) + ".lproj",
    wxStandardPaths::Get()
      .GetLocalizedResourcesDir(
        wxUILocale::GetLanguageCanonicalName(language),
        wxStandardPaths::ResourceCat_Messages)
      .ToStdString(),
    wxStandardPaths::Get().GetResourcesDir().ToStdString()};

  for (const auto& dir : dirs)
  {
    if (fs::is_directory(dir))
    {
      bool found = false;

      for (const auto& p : fs::directory_iterator(dir))
      {
        if (
          fs::is_regular_file(p.path()) &&
          matches_one_of(p.path().filename().string(), "*.mo"))
        {
          m_catalog_dir  = dir;
          m_catalog_file = p.path().string();
          found          = true;

          if (!wxTranslations::Get()->AddCatalog(p.path().stem().string()))
          {
            log("could not add catalog") << p.path().stem().string();
          }
        }
      }

      if (found)
      {
        return;
      }
    }
  }

  log::trace("no locale files")
    << std::accumulate(dirs.begin(), dirs.end(), std::string());
}

wxMsgCatalog* wex::file_translations_loader::LoadCatalog(
  const wxString& domain,
  const wxString& lang)
{
  if (m_catalog_file.empty())
  {
    return nullptr;
  }

  log::trace("loaded catalog") << m_catalog_file;
  return wxMsgCatalog::CreateFromFile(m_catalog_file, domain);
}
