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

void wex::app::add_catalogs() const
{
  wxLogNull logNo; // prevent wxLog

  if (const auto& dir(get_catalog_dir());
      fs::is_directory(dir) && get_language() != wxLANGUAGE_DEFAULT)
  {
    for (const auto& p : fs::recursive_directory_iterator(dir))
    {
      if (
        fs::is_regular_file(p.path()) &&
        matches_one_of(p.path().filename().string(), "*.mo"))
      {
        if (!wxTranslations::Get()->AddCatalog(p.path().stem().string()))
        {
          log("could not add catalog") << p.path().stem().string();
        }
      }
    }
  }
}

const std::string wex::app::get_catalog_dir() const
{
  const std::string dir(wxStandardPaths::Get().GetLocalizedResourcesDir(
    get_locale().GetLanguageCanonicalName(get_language())
#ifndef __WXMSW__
      ,
    wxStandardPaths::ResourceCat_Messages
#endif
    ));

  if (!fs::is_directory(dir) && get_language() != wxLANGUAGE_DEFAULT)
  {
    log("missing locale files for")
      << get_locale().GetName().ToStdString() << "in" << dir;
  }

  return dir;
}

wxLanguage wex::app::get_language() const
{
  const wxLanguageInfo* info = nullptr;

  if (const auto& lang(config("Language").get()); !lang.empty())
  {
    if ((info = wxUILocale::FindLanguageInfo(lang)) == nullptr)
    {
      log("unknown language") << lang;
    }
  }

  const auto lang =
    (info != nullptr ? (wxLanguage)info->Language : wxLANGUAGE_DEFAULT);

  return lang;
}

const wxUILocale& wex::app::get_locale()
{
  return wxUILocale::GetCurrent();
}

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

  if (!wxUILocale::UseDefault())
  {
    log("could not set locale");
  }

  // Construct translation, from now on things will be translated.
  wxTranslations::Set(new wxTranslations());
  wxTranslations::Get()->SetLanguage(get_language());

  add_catalogs();

  // Necessary for auto_complete images.
  wxInitAllImageHandlers();

  wxTheClipboard->UsePrimarySelection(true);

  return true; // do not call base class: we have our own cmd line processing
}
