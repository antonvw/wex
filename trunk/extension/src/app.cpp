/******************************************************************************\
* File:          app.cpp
* Purpose:       Implementation of wxExApp class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/config.h>
#include <wx/dir.h>
#include <wx/stdpaths.h>
#include <wx/extension/app.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/printing.h>
#include <wx/extension/stc.h>
#include <wx/extension/tool.h>

int wxExApp::OnExit()
{
  delete wxExFindReplaceData::Set(NULL);
  delete wxExLexers::Set(NULL);
  delete wxExLog::Set(NULL);
  delete wxExPrinting::Set(NULL);

  return wxApp::OnExit();
}

bool wxExApp::OnInit()
{
  // Init the localization, from now on things will be translated.
  // So do this before constructing config and wxExTool::Initialize, as these use localization.
  if (m_Locale.Init())
  {
    // If there are catalogs in the catalog_dir, then add them to the m_Locale.
    // README: We use the canonical name, also for windows, not sure whether that is
    // the best.
    const wxString catalogDir = wxStandardPaths::Get().GetLocalizedResourcesDir(
      m_Locale.GetCanonicalName(),
      // This seems to be necessarty for wxGTK. For wxMSW it isn't used.
      wxStandardPaths::ResourceCat_Messages);

    if (wxFileName::DirExists(catalogDir))
    {
      wxArrayString files;
      wxDir::GetAllFiles(catalogDir, &files);

      for (size_t i = 0 ; i < files.GetCount(); i++)
      {
        // Default the wxstd is already loaded by m_Locale.Init(),
        // so do not do it twice.
        const wxFileName fn(files.Item(i));

        if (!m_Locale.IsLoaded(fn.GetName()))
        {
          if (!m_Locale.AddCatalog(fn.GetName()))
          {
            wxLogError("Catalog could not be added: " + fn.GetName());
          }
        }
      }
    }
  }

  // Now construct the config, as most classes use it.
  wxConfigBase* config;
#ifdef wxExUSE_PORTABLE
  config = new wxFileConfig(
    wxEmptyString,
    wxEmptyString,
    wxFileName(
      wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath(),
      GetAppName() + wxString(".cfg")).GetFullPath(),
    wxEmptyString,
    wxCONFIG_USE_LOCAL_FILE);
#else
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

  // Finally call all available static initializers.
  wxExTool::Initialize();
  wxExSTC::PathListInit();

  return wxApp::OnInit();
}
