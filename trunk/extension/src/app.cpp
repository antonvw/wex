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
#include <wx/intl.h> // for wxLocale
#include <wx/stdpaths.h>
#include <wx/extension/app.h>
#include <wx/extension/frd.h>
#include <wx/extension/lexers.h>
#include <wx/extension/log.h>
#include <wx/extension/stc.h>
#include <wx/extension/tool.h>
#include <wx/extension/util.h>

wxString wxExApp::m_CatalogDir;
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
wxHtmlEasyPrinting* wxExApp::m_Printer = NULL;
#endif

int wxExApp::OnExit()
{
#if wxUSE_GUI
  wxExSTC::CleanUp();
#endif

  wxExFindReplaceData::Destroy();
  wxExLexers::Destroy();
  wxExLog::Destroy();

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  delete m_Printer;
#endif

  return wxApp::OnExit();
}

bool wxExApp::OnInit()
{
  // Init the localization, from now on things will be translated.
  // So do this before constructing config and wxExTool::Initialize, as these use localization.
  wxLocale locale;

  if (locale.Init())
  {
    // If there are catalogs in the catalog_dir, then add them to the locale.
    // README: We use the canonical name, also for windows, not sure whether that is
    // the best.
    m_CatalogDir = wxStandardPaths::Get().GetLocalizedResourcesDir(
      locale.GetCanonicalName(),
      // This seems to be necessarty for wxGTK. For wxMSW it isn't used.
      wxStandardPaths::ResourceCat_Messages);

    if (wxFileName::DirExists(m_CatalogDir))
    {
      wxArrayString files;
      wxDir::GetAllFiles(m_CatalogDir, &files);

      for (size_t i = 0 ; i < files.GetCount(); i++)
      {
        // Default the wxstd is already loaded by locale.Init(),
        // so do not do it twice.
        const wxFileName fn(files.Item(i));

        if (!locale.IsLoaded(fn.GetName()))
        {
          if (!locale.AddCatalog(fn.GetName()))
          {
            wxLogError("Catalog could not be added: " + fn.GetName());
          }
        }
      }
    }
  }
  else
  {
    m_CatalogDir = wxString::Format(
      "Could not initialize locale name: %s canonical name: %s",
      locale.GetName(),
      locale.GetCanonicalName ());
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

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  m_Printer = new wxHtmlEasyPrinting();

  m_Printer->SetFonts(wxEmptyString, wxEmptyString); // use defaults
  m_Printer->GetPageSetupData()->SetMarginBottomRight(wxPoint(15, 5));
  m_Printer->GetPageSetupData()->SetMarginTopLeft(wxPoint(15, 5));

  m_Printer->SetHeader(wxExPrintHeader(wxFileName()));
  m_Printer->SetFooter(wxExPrintFooter());
#endif

  // Finally call all available static initializers.
  wxExTool::Initialize();
  wxExSTC::PathListInit();

  return wxApp::OnInit();
}

void wxExApp::ToggleConfig(const wxString& key)
{
  const bool val = wxConfigBase::Get()->ReadBool(key, true);
  wxConfigBase::Get()->Write(key, !val);
}
