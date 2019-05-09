////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex::app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <filesystem>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stdpaths.h>
#include <wex/app.h>
#include <wex/addressrange.h>
#include <wex/config.h>
#include <wex/ex.h>
#include <wex/ctags.h>
#include <wex/frd.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/printing.h>
#include <wex/stc.h>
#include <wex/util.h>
#include <wex/vcs.h>
#include <wex/version.h>
#include <wex/vi-macros.h>

#define NO_ASSERT 1

namespace fs = std::filesystem;

void wex::app::OnAssertFailure(
  const wxChar* file, int line, const wxChar* func, 
  const wxChar* cond, const wxChar* msg)
{
#ifdef NO_ASSERT
  log("OnAssertFailure") << 
    "file:" << file << "line:" << line << "func:" << func << 
    "cond:" << cond << "msg:" << msg;
#else
  wxApp::OnAssertFailure(file, line, func, cond, msg);
#endif
}
    
int wex::app::OnExit()
{
  delete find_replace_data::set(nullptr);
  delete lexers::set(nullptr);
  delete printing::set(nullptr);
  
  addressrange::on_exit();
  ctags::close();
  stc::on_exit();
  log::verbose(1) << "exit";

  return wxApp::OnExit(); // this destroys the config
}

bool wex::app::OnInit()
{
  config::init();

  log::init(argc, argv);
  log::verbose(1) << "started:" << GetAppName().ToStdString() << get_version_info().get();

  const wxLanguageInfo* info = nullptr;
  
  if (config("LANG").exists())
  {
    if ((info = wxLocale::FindLanguageInfo(
      config("LANG").get())) == nullptr)
    {
      log() << "unknown language:" << config("LANG").get();
    }
  }
    
  // Init the localization, from now on things will be translated.
  // Do not load wxstd, we load all files ourselves,
  // and do not want messages about loading non existing wxstd files.
  if (const auto lang = (info != nullptr ? info->Language: wxLANGUAGE_DEFAULT);
    !m_Locale.Init(lang, wxLOCALE_DONT_LOAD_DEFAULT))
  {
    log::verbose("could not init locale for") << 
      (!wxLocale::GetLanguageName(lang).empty() ?
        wxLocale::GetLanguageName(lang).ToStdString(): 
        std::to_string(lang));
  }
  else
  {
    // If there are catalogs in the catalog_dir, then add them to the m_Locale.
    // README: We use the canonical name, also for windows, not sure whether that is
    // the best.
    m_CatalogDir = wxStandardPaths::Get().GetLocalizedResourcesDir(
      m_Locale.GetCanonicalName()
#ifndef __WXMSW__
      , wxStandardPaths::ResourceCat_Messages
#endif
      ).ToStdString();

    if (fs::is_directory(m_CatalogDir))
    {
      for (const auto& p: fs::recursive_directory_iterator(m_CatalogDir))
      {
        if (fs::is_regular_file(p.path()) && 
          matches_one_of(p.path().filename().string(), "*.mo"))
        {
          if (!m_Locale.AddCatalog(p.path().stem().string()))
          {
            log() << "could not add catalog:" << p.path().stem().string();
          }
        }
      }
    }
    else if (info != nullptr)
    {
      log("missing locale files for") << m_Locale.GetName().ToStdString();
    }
  }

  // Necessary for autocomplete images.
  wxInitAllImageHandlers();

  stc::on_init();
  vcs::load_document();
  vi_macros().load_document();

  return true; // wxApp::OnInit(); // we have our own cmd line processing
}
