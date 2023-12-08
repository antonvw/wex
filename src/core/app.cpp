////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/app.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/core/version.h>
#include <wx/clipbrd.h>

#ifdef __WXGTK__
#include <X11/Xlib.h>
#include <thread>
#endif

#include <iostream>

#include "app-locale.h"

namespace wex
{
// This routine performs first init of  a wex::app,
// it runs before OnInit.
int first_init()
{
#ifdef __WXGTK__
  XInitThreads();
#endif

  return 1;
}
} // namespace wex

int wex::app::m_first_init = first_init();

const std::string wex::app::get_catalog_dir() const
{
  return file_translations_loader::catalog_dir();
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

  delete wxTranslations::Get();
  delete m_loader;

  return wxApp::OnExit();
}

bool wex::app::OnInit()
{
  log::on_init();
  log::info("started") << GetAppName().ToStdString()
                       << get_version_info().get(false);

  config::on_init();

  // Construct translation, from now on things will be translated.
  set_language();

  if (m_language != wxLANGUAGE_UNKNOWN && m_language != wxLANGUAGE_DEFAULT)
  {
    m_loader = new file_translations_loader();
    wxUILocale::FromTag(wxUILocale::GetLanguageCanonicalName(m_language));
    wxTranslations::Set(new wxTranslations());
    wxTranslations::Get()->SetLanguage(m_language);
    wxTranslations::Get()->SetLoader(m_loader);

    m_loader->add_catalogs(m_language);
  }

  // Necessary for auto_complete images.
  wxInitAllImageHandlers();

  wxTheClipboard->UsePrimarySelection(true);

  return true; // do not call base class: we have our own cmd line processing
}

wxLanguage wex::app::get_default_language() const
{
  return wxLANGUAGE_DEFAULT;
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
      log::trace("canonical name")
        << wxUILocale::GetLanguageCanonicalName(m_language).ToStdString();
    }
  }
  else
  {
    m_language = get_default_language();
  }
}
