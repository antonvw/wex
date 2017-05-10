////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wxExApp class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dir.h>
#include <wx/fileconf.h> 
#include <wx/stdpaths.h>
#include <wx/extension/app.h>
#include <wx/extension/addressrange.h>
#include <wx/extension/ex.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>
#include <wx/extension/vcs.h>

#define NO_ASSERT 1

void wxExApp::OnAssertFailure(
  const wxChar* file, int line, const wxChar* func, 
  const wxChar* cond, const wxChar* msg)
{
#ifdef NO_ASSERT
  std::wcout << wxNow() << ": OnAssertFailure: file: " << file << 
    " line: " << line << " func: " << func << 
    " cond: " << cond << " msg: " << msg << "\n";
#else
  wxApp::OnAssertFailure(file, line, func, cond, msg);
#endif
}
    
int wxExApp::OnExit()
{
  delete wxExFindReplaceData::Set(nullptr);
  delete wxExLexers::Set(nullptr);
  delete wxExPrinting::Set(nullptr);

  wxExAddressRange::Cleanup();
  wxExEx::Cleanup();

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
      std::cout << "Unknown language: " << 
        wxConfigBase::Get()->Read("LANG").ToStdString() << "\n";;
    }
  }
  
  const int lang = (info != nullptr ? info->Language: wxLANGUAGE_DEFAULT); 
    
  // Init the localization, from now on things will be translated.
  // Do not load wxstd, we load all files ourselved,
  // and do not want messages about loading non existing wxstd files.
  if (!m_Locale.Init(lang, wxLOCALE_DONT_LOAD_DEFAULT) &&
    !wxLocale::GetLanguageName(lang).empty())
  {
    std::cout << "Could not init locale for: " << wxLocale::GetLanguageName(lang) << "\n";
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
        std::cout << "Could not add catalog: " << 
          wxFileName(it).GetName(). ToStdString() << "\n";;
      }
    }
  }
  else if (info != nullptr)
  {
    std::cout << "Missing locale files for: " << GetLocale().GetName() << "\n";
  }

  wxExVCS::LoadDocument();

  return true; // wxApp::OnInit(); // we have our own cmd line processing
}
