////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2025 Anton van Wezenbeek
// ////////////////////////////////////////////////////////////////////////////////

#include <wex/core/app.h>
#include <wex/core/config.h>
#include <wex/core/log.h>
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
  log::on_init();

  config::on_init();

  set_language();

  if (m_language != wxLANGUAGE_UNKNOWN && m_language != wxLANGUAGE_DEFAULT)
  {
    if (!wxUILocale::UseDefault())
    {
      log("failed to initialize the default system locale");
    }

    // Construct translation, from now on strings will be translated.
    auto* trans = new translations();
    wxTranslations::Set(trans);
    trans->SetLanguage(m_language);

    trans->add_catalogs(m_language);
  }

  wxTheClipboard->UsePrimarySelection(true);

  SetAppearance(Appearance::Dark);

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
      log::trace("canonical name")
        << wxUILocale::GetLanguageCanonicalName(m_language).ToStdString();
    }
  }
  else
  {
    m_language = wxLANGUAGE_DEFAULT;
  }
}
