/******************************************************************************\
* File:          app.h
* Purpose:       Include file for exApp class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXAPP_H
#define _EXAPP_H

#include <wx/intl.h> // for wxLocale
#include <wx/html/htmprint.h>
#include <wx/extension/config.h>
#include <wx/extension/lexers.h>

/// Offers the application, with a configuration, lexer, printer, logging
/// and locale.
/// Your application should be derived from this class.
class exApp : public wxApp
{
public:
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
  static exConfig* GetConfig() {return m_Config;};

  /// Gets the lexers.
  static exLexers* GetLexers() {return m_Lexers;};

  /// Gets the locale.
  static wxLocale& GetLocale() {return m_Locale;};

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  /// Gets the printer.
  static wxHtmlEasyPrinting* GetPrinter() {return m_Printer;};
#endif

  /// Logs text (only if SetLogging() is called, default it is off).
  static void Log(const wxString& text);

  /// Sets key as a long.
  static void SetConfig(const wxString& key, long value)
    {m_Config->Set(key, value);}

  /// Sets key as a string.
  static void SetConfig(const wxString& key, const wxString& value)
    {m_Config->Set(key, value);}

  /// Sets key as a bool.
  static void SetConfigBool(const wxString& key, bool value = true)
    {m_Config->SetBool(key, value);}

  /// Sets logging as specified.
  static void SetLogging(bool logging = true) {m_Logging = logging;};

  /// Toggles boolean key.
  static void ToggleConfig(const wxString& key)
    {m_Config->Toggle(key);}

protected:
  // Interface from wxApp.
  /// Constructs the config, lexers and printer (and reads the lexers).
  /// Initializes the locale and exTool.
  /// In your class first set the app name, as it uses this name for the config file.
  virtual bool OnInit();

  /// This destroys (and so writes) the config, lexers, printer
  /// and cleans up things if necessary.
  virtual int OnExit();
private:
  static bool m_Logging;
  static exConfig* m_Config;
  static exLexers* m_Lexers;
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  static wxHtmlEasyPrinting* m_Printer;
#endif

  static wxLocale m_Locale; ///< your locale
  static wxString m_CatalogDir;
};
#endif
