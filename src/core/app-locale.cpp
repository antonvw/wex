////////////////////////////////////////////////////////////////////////////////
// Name:      app-locale.cpp
// Purpose:   Implementation of wex::translations class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/log.h>

#include "app-locale.h"

void wex::translations::add_catalogs(wxLanguage language)
{
  wxFileTranslationsLoader::AddCatalogLookupPathPrefix(".");

  const std::vector<std::string> files{"wxstd", "wex"};

  for (const auto& file : files)
  {
    if (!AddAvailableCatalog(file, language))
    {
      log("no catalog file") << file;
    }
    else
    {
      log::trace("added catalog file") << file;
    }
  }
}
