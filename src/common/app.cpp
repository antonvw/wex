////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>

#include <wx/clipbrd.h>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/app.h>
#include <wex/config.h>
#include <wex/core.h>
#include <wex/frd.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/printing.h>
#include <wex/version.h>
#include <wx/stdpaths.h>

namespace fs = std::filesystem;

int wex::app::OnExit()
{
  delete find_replace_data::set(nullptr);
  delete lexers::set(nullptr);
  delete printing::set(nullptr);

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

  if (config("LANG").exists())
  {
    if ((info = wxLocale::FindLanguageInfo(config("LANG").get())) == nullptr)
    {
      log("unknown language") << config("LANG").get();
    }
  }

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
    // If there are catalogs in the catalog_dir, then add them to the m_locale.
    // README: We use the canonical name, also for windows, not sure whether
    // that is the best.
    m_catalog_dir = wxStandardPaths::Get().GetLocalizedResourcesDir(
      m_locale.GetCanonicalName()
#ifndef __WXMSW__
        ,
      wxStandardPaths::ResourceCat_Messages
#endif
    );

    if (fs::is_directory(m_catalog_dir))
    {
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
      log("missing locale files for") << m_locale.GetName().ToStdString();
    }
  }

  // Necessary for auto_complete images.
  wxInitAllImageHandlers();

  wxTheClipboard->UsePrimarySelection(true);

  return true; // do not call base class: we have our own cmd line processing
}
