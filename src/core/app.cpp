////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/app.h>
#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/core/log.h>
#include <wex/core/version.h>
#include <wx/clipbrd.h>
#include <wx/stdpaths.h>

#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int wex::app::OnExit()
{
  try
  {
    config::on_exit();

    log::info("exit");
  }
  catch (std::exception& e)
  {
    std::cout << e.what() << "\n";
  }

  return wxApp::OnExit();
}

bool wex::app::OnInit()
{
  log::init();
  log::info("started") << GetAppName().ToStdString()
                       << get_version_info().get();

  config::on_init();

  const wxLanguageInfo* info = nullptr;

  if (const auto& lang(config("Language").get()); !lang.empty())
  {
    if ((info = wxLocale::FindLanguageInfo(lang)) == nullptr)
    {
      log("unknown language") << lang;
    }
  }

  wxLogNull logNo; // prevent wxLog

  // Init the localization, from now on things will be translated.
  // Do not load wxstd, we load all files ourselves,
  // and do not want messages about loading non existing wxstd files.
  if (const auto lang = (info != nullptr ? info->Language : wxLANGUAGE_DEFAULT);
      !m_locale.Init(lang, wxLOCALE_DONT_LOAD_DEFAULT))
  {
    log::debug("could not init locale for")
      << (!wxLocale::GetLanguageName(lang).empty() ?
            wxLocale::GetLanguageName(lang).ToStdString() :
            std::to_string(lang));
  }
  else
  {
    m_catalog_dir = "/usr/local/share/locale/" +
                    m_locale.GetName().ToStdString() + "/LC_MESSAGES";

    if (!fs::is_directory(m_catalog_dir))
    {
      m_catalog_dir = wxStandardPaths::Get().GetLocalizedResourcesDir(
        m_locale.GetCanonicalName()
#ifndef __WXMSW__
          ,
        wxStandardPaths::ResourceCat_Messages
#endif
      );
    }

    if (fs::is_directory(m_catalog_dir))
    {
      // If there are catalogs in the catalog_dir, then add them to the
      // m_locale.
      for (const auto& p : fs::recursive_directory_iterator(m_catalog_dir))
      {
        if (
          fs::is_regular_file(p.path()) &&
          matches_one_of(p.path().filename().string(), "*.mo"))
        {
          if (!m_locale.AddCatalog(p.path().stem().string()))
          {
            log("could not add catalog") << p.path().stem().string();
          }
        }
      }
    }
    else if (info != nullptr)
    {
      log("missing locale files for")
        << m_locale.GetName().ToStdString() << m_catalog_dir;
    }
  }

  // Necessary for auto_complete images.
  wxInitAllImageHandlers();

  wxTheClipboard->UsePrimarySelection(true);

  return true; // do not call base class: we have our own cmd line processing
}
