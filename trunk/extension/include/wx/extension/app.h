/******************************************************************************\
* File:          app.h
* Purpose:       Include file for wxExApp class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXAPP_H
#define _EXAPP_H

#include <wx/app.h>
#include <wx/intl.h> // for wxLocale
#include <wx/html/htmprint.h>
#include <wx/config.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExLog

class wxExFindReplaceData;

/// Offers the application, with a configuration, lexer, printer, logging
/// and locale.
/// Your application should be derived from this class.
class wxExApp : public wxApp
{
public:
  /// Gets the dir where the catalogs are for the locale.
  static wxString& GetCatalogDir() {return m_CatalogDir;};

  /// Gets the find replace data.
  static wxExFindReplaceData* GetFindReplaceData() {return m_FindReplaceData;};

  /// Gets the lexers.
  static wxExLexers* GetLexers() {return m_Lexers;};

  /// Gets the locale.
  static wxLocale& GetLocale() {return m_Locale;};

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  /// Gets the printer.
  static wxHtmlEasyPrinting* GetPrinter() {return m_Printer;};
#endif

  /// Logs text (only if SetLogging(true) is called, default it is off).
  /// Returns true if logging was on and write was successfull.
  static bool Log(const wxString& text) {
    if (m_Logging) return wxExLog(text);
    else           return false;};

  /// Constructs the config, lexers and printer (and reads the lexers).
  /// Initializes the locale and wxExTool.
  /// In your class first set the app name, as it uses this name for the config file.
  /// See for documentation the lexers.xml file.
  virtual bool OnInit();

  /// This destroys (and so writes) the config, lexers, printer
  /// and cleans up things if necessary.
  virtual int OnExit();

  /// Sets logging as specified.
  /// If the logging is true and the logfile does not exist, it is created.
  static bool SetLogging(bool logging = true);

  /// Toggles boolean key.
  static void ToggleConfig(const wxString& key);
private:
  static bool m_Logging;
  static wxConfigBase* m_Config;
  static wxExFindReplaceData* m_FindReplaceData;
  static wxExLexers* m_Lexers;
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  static wxHtmlEasyPrinting* m_Printer;
#endif

  static wxLocale m_Locale; ///< your locale
  static wxString m_CatalogDir;
};
#endif
