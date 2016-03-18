////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wxExApp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dir.h>
#include <wx/fileconf.h> 
#include <wx/stdpaths.h>
#include <wx/extension/app.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

int wxExApp::OnExit()
{
  delete wxExFindReplaceData::Set(nullptr);
  delete wxExLexers::Set(nullptr);
  delete wxExPrinting::Set(nullptr);

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
  
  const wxLanguageInfo* info = nullptr;
  
  if (wxConfigBase::Get()->Exists("LANG"))
  {
    info = wxLocale::FindLanguageInfo(wxConfigBase::Get()->Read("LANG"));
    
    if (info == nullptr)
    {
      wxLogMessage("Unknown language: " + wxConfigBase::Get()->Read("LANG"));
    }
  }
  
  const int lang = (info != nullptr ? info->Language: wxLANGUAGE_DEFAULT); 
    
  // Init the localization, from now on things will be translated.
  // Do not load wxstd, we load all files ourselved,
  // and do not want messages about loading non existing wxstd files.
  if (!m_Locale.Init(lang, wxLOCALE_DONT_LOAD_DEFAULT))
  {
    wxLogMessage("Could not init locale for: " + GetLocale().GetName());
  }
  
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

    for (const auto& it : files)
    {
      if (!m_Locale.AddCatalog(wxFileName(it).GetName()))
      {
        wxLogMessage("Could not add catalog: " + wxFileName(it).GetName());
      }
    }
  }
  else if (info != nullptr)
  {
    wxLogMessage("Missing locale files for: " + GetLocale().GetName());
  }

  wxExVCS::LoadDocument();

  return true; // wxApp::OnInit(); // we have our own cmd line processing
}
