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

  if (const auto& dir(get_catalog_dir()); fs::is_directory(dir))
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
  else
  {
    log("missing locale files for")
      << get_locale().GetName().ToStdString() << "in" << dir;
  }
}

const std::string wex::app::get_catalog_dir() const
{
  return wxStandardPaths::Get().GetResourcesDir();
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
  set_language();
  wxTranslations::Set(new wxTranslations());
  wxTranslations::Get()->SetLanguage(m_language);

  if (m_language != wxLANGUAGE_UNKNOWN && m_language != wxLANGUAGE_DEFAULT)
  {
    add_catalogs();
  }

  // Necessary for auto_complete images.
  wxInitAllImageHandlers();

  wxTheClipboard->UsePrimarySelection(true);

  return true; // do not call base class: we have our own cmd line processing
}

void wex::app::set_language()
{
  if (const auto& lang(config("Language").get()); !lang.empty())
  {
    if (const auto* info = wxUILocale::FindLanguageInfo(lang); info == nullptr)
    {
      log("unknown language") << lang;
      m_language = wxLANGUAGE_UNKNOWN;
    }
    else
    {
      m_language = (wxLanguage)info->Language;
    }
  }
  else
  {
    m_language = wxLANGUAGE_DEFAULT;
  }
}
