////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wxExApp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#ifdef wxExUSE_PORTABLE
#include <wx/fileconf.h> 
#endif
#include <wx/extension/app.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/vcs.h>

int wxExApp::OnExit()
{
  delete wxExFindReplaceData::Set(NULL);
  delete wxExLexers::Set(NULL);
  delete wxExPrinting::Set(NULL);

  return wxApp::OnExit(); // this destroys the config
}

bool wxExApp::OnInit()
{
  wxConfigBase* config;
#ifdef wxExUSE_PORTABLE
  // Use a portable file config.
  // This should be before first use of wxConfigBase::Get().
  config = new wxFileConfig(
    wxEmptyString,
    wxEmptyString,
    wxFileName(
      wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath(),
      GetAppName().Lower() + wxString(".ini")).GetFullPath(),
    wxEmptyString,
    wxCONFIG_USE_LOCAL_FILE);
#else
  // Remember, this one is used on Linux.
  // As wxStandardPaths::GetUserDataDir is used, subdir is necessary for config.
  // (ignored on non-Unix system)
  config = new wxConfig(
    wxEmptyString,
    wxEmptyString,
    wxEmptyString,
    wxEmptyString,
    wxCONFIG_USE_LOCAL_FILE | wxCONFIG_USE_SUBDIR);
#endif
  wxConfigBase::Set(config);
  
  const int lang = (wxConfigBase::Get()->Exists("LANG") ?
    wxConfigBase::Get()->ReadLong("LANG", wxLANGUAGE_DEFAULT):
    wxLANGUAGE_DEFAULT);
    
  // Init the localization, from now on things will be translated.
  // Do not load wxstd, we load all files ourselved,
  // and do not want messages about loading non existing wxstd files.
  m_Locale.Init(lang, wxLOCALE_DONT_LOAD_DEFAULT);
  
  // If there are catalogs in the catalog_dir, then add them to the m_Locale.
  // README: We use the canonical name, also for windows, not sure whether that is
  // the best.
  m_CatalogDir = wxStandardPaths::Get().GetLocalizedResourcesDir(
    m_Locale.GetCanonicalName(),
    // This seems to be necessary for wxGTK. For wxMSW it isn't used.
    wxStandardPaths::ResourceCat_Messages);

  if (wxFileName::DirExists(m_CatalogDir))
  {
    wxArrayString files;
    wxDir::GetAllFiles(m_CatalogDir, &files, "*.mo");

    for (auto it : files)
    {
      const wxFileName fn(it);

      if (!m_Locale.AddCatalog(fn.GetName()))
      {
        wxLogError("Catalog could not be added: " + fn.GetName());
      }
    }
  }

  wxExVCS::LoadDocument();

  return wxApp::OnInit();
}
