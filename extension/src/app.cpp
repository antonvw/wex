////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wxExApp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <experimental/filesystem>

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/fileconf.h> 
#include <wx/stdpaths.h>
#include <wx/extension/app.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/printing.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>
#include <wx/extension/version.h>
#include <wx/extension/vi-macros.h>
#include <easylogging++.h>

#define NO_ASSERT 1

INITIALIZE_EASYLOGGINGPP

namespace fs = std::experimental::filesystem;

void wxExApp::OnAssertFailure(
  const wxChar* file, int line, const wxChar* func, 
  const wxChar* cond, const wxChar* msg)
{
#ifdef NO_ASSERT
  LOG(ERROR) << "OnAssertFailure: file: " << file << 
    " line: " << line << " func: " << func << 
    " cond: " << cond << " msg: " << msg;
#else
  wxApp::OnAssertFailure(file, line, func, cond, msg);
#endif
}
    
int wxExApp::OnExit()
{
  delete wxExFindReplaceData::Set(nullptr);
  delete wxExLexers::Set(nullptr);
  delete wxExPrinting::Set(nullptr);

  wxExAddressRange::OnExit();
  wxExSTC::OnExit();

  VLOG(1) << "exit";

  return wxApp::OnExit(); // this destroys the config
}

bool wxExApp::OnInit()
{
  // This should be before first use of wxConfigBase::Get().
  wxConfigBase::Set(new wxFileConfig(wxEmptyString, wxEmptyString,
    wxFileName(wxExConfigDir(), GetAppName().Lower() + 
#ifdef __WXMSW__
    ".ini"
#else
    ".conf"
#endif
      ).GetFullPath(), wxEmptyString, wxCONFIG_USE_LOCAL_FILE));

  // Load elp configuration from file.
  const wxExPath elp(wxExConfigDir(), "conf.elp");

  if (elp.FileExists())
  {
    el::Loggers::reconfigureAllLoggers(el::Configurations(elp.Path().string()));
  }

  // We need to convert argc and argv, as elp expects = sign between values.
  // The logging-flags are handled by syncped.
  int argc_elp = 0;
  char *argv_elp[20]; // argc does not compile under MSW
  const std::vector <std::pair<
    std::string, std::string>> supported {
      {"-m", "-vmodule"},
      {"-D", "--default-log-file"},
      {"-L", "--loggingflags"},
      {"--logfile", "--default-log-file"},
      {"--logflags", "--loggingflags"},
      {"--x", "--v"}, // for testing with verbosity
      {"--v", "--v"}};

  for (int i = 0; i < argc; i++)
  {
    bool found = false;

    for (const auto& s : supported)
    {
      if (strcmp(argv[i].c_str(), s.first.c_str()) == 0)
      {
        found = true;
        char* buffer = (char *)malloc(strlen(argv[i + 1].c_str()));
        strcpy(buffer, argv[i + 1].c_str());

        argv_elp[argc_elp] = (char *)malloc(s.second.size() + 1 + strlen(argv[i + 1].c_str()));
        sprintf(argv_elp[argc_elp], "%s=%s", s.second.c_str(), buffer);
        argc_elp++;
        i++;
        free(buffer);
      }
    }

    if (!found)
    {
      argv_elp[argc_elp] = (char *)malloc(strlen(argv[i].c_str()));
      strcpy(argv_elp[argc_elp], argv[i].c_str());
      argc_elp++;
    }
  }

  START_EASYLOGGINGPP(argc_elp, argv_elp);

#ifndef __WXMSW__
  for (int i = 0; i < argc_elp; i++)
  {
    free(argv_elp[i]);
  }
#endif

  VLOG(1) 
    << "started: " 
    << GetAppName() << "-" << wxExGetVersionInfo().GetVersionOnlyString().c_str()
    << " verbosity: " 
    << el::Loggers::verboseLevel()
    << " config: " << elp.Path().string();

  const wxLanguageInfo* info = nullptr;
  
  if (wxConfigBase::Get()->Exists("LANG"))
  {
    if ((info = wxLocale::FindLanguageInfo(
      wxConfigBase::Get()->Read("LANG"))) == nullptr)
    {
      wxExLog() << "unknown language:" << wxConfigBase::Get()->Read("LANG");
    }
  }
    
  // Init the localization, from now on things will be translated.
  // Do not load wxstd, we load all files ourselves,
  // and do not want messages about loading non existing wxstd files.
  if (const auto lang = (info != nullptr ? info->Language: wxLANGUAGE_DEFAULT);
    !m_Locale.Init(lang, wxLOCALE_DONT_LOAD_DEFAULT))
  {
    VLOG(9) << "could not init locale for: " << 
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
          wxExMatchesOneOf(p.path().filename().string(), "*.mo"))
        {
          if (!m_Locale.AddCatalog(p.path().stem().string()))
          {
            wxExLog() << "could not add catalog:" << p.path().stem().string();
          }
        }
      }
    }
    else if (info != nullptr)
    {
      wxExLog() << "missing locale files for:" << m_Locale.GetName();
    }
  }

  // Necessary for autocomplete images.
  wxInitAllImageHandlers();

  wxExSTC::OnInit();
  wxExVCS::LoadDocument();
  wxExViMacros().LoadDocument();

  return true; // wxApp::OnInit(); // we have our own cmd line processing
}
