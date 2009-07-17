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
#include <wx/extension/config.h>
#include <wx/extension/lexers.h>
#include <wx/extension/util.h> // for wxExLog

/// Offers the application, with a configuration, lexer, printer, logging
/// and locale.
/// Your application should be derived from this class.
class wxExApp : public wxApp
{
public:
  /// Constructs the config, lexers and printer (and reads the lexers).
  /// Initializes the locale and wxExTool.
  /// In your class first set the app name, as it uses this name for the config file.
  /// See for documentation the lexers.xml file.
  virtual bool OnInit();

  /// This destroys (and so writes) the config, lexers, printer
  /// and cleans up things if necessary.
  virtual int OnExit();

  /// Gets the dir where the catalogs are for the locale.
  static wxString& GetCatalogDir() {return m_CatalogDir;};

  /// Gets key as a long.
  static long GetConfig(const wxString& key, long default_value)
    {return m_Config->Get(key, default_value);}

  /// Gets key as a string.
  static const wxString GetConfig(const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',')
    {return m_Config->Get(key, default_value, field_separator);}

  /// Gets key as a bool.
  static bool GetConfigBool(const wxString& key, bool default_value = true)
    {return m_Config->GetBool(key, default_value);}

  /// Gets the config.
  static wxExConfig* GetConfig() {return m_Config;};

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

  /// Sets key.
  static void SetConfig(const wxString& key, const wxVariant& value)
    {m_Config->Set(key, value);}

  /// Sets logging as specified.
  /// If the logging is true and the logfile does not exist, it is created.
  static bool SetLogging(bool logging = true);

  /// Toggles boolean key.
  static void ToggleConfig(const wxString& key)
    {m_Config->Toggle(key);}
private:
  static bool m_Logging;
  static wxExConfig* m_Config;
  static wxExLexers* m_Lexers;
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  static wxHtmlEasyPrinting* m_Printer;
#endif

  static wxLocale m_Locale; ///< your locale
  static wxString m_CatalogDir;
};
#endif
