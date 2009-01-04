/******************************************************************************\
* File:          extension.h
* Purpose:       Include file for most wxWidgets extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _WXEXTENSION_H
#define _WXEXTENSION_H

#include <wx/intl.h> // for wxLocale
#include <wx/extension/version.h>
#include <wx/extension/base.h>
#include <wx/extension/lexers.h>
#include <wx/extension/config.h>
#include <wx/extension/file.h>

// Log methods.
/// Returns the filename of the logfile.
const exFileName exLogfileName();
/// Logs text with a timestamp at the end of the file.
void exLog(const wxString& text, const exFileName& filename = exLogfileName());

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
  static void Log(const wxString& text) {
    if (m_Logging) exLog(text);}

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

/// SVN types supported.
enum exSvnType
{ 
  SVN_CAT,    ///< svn cat
  SVN_COMMIT, ///< svn commit 
  SVN_DIFF,   ///< svn diff 
  SVN_INFO,   ///< svn info
  SVN_LOG,    ///< svn log 
  SVN_STAT,   ///< svn stat 
};

#if wxUSE_GUI
/// This class collects all svn handling.
class exSVN
{
public:
  /// Constructor, specify the type of what to get.
  exSVN(exSvnType m_Type);

  /// Gets info from svn.
  /// If no fullpath is specified, a dialog with base folder is shown, otherwise
  /// the specified fullpath is used for getting svn contents from.
  /// Returns -1 if dialog was cancelled, 0 if okay, or the number of errors 
  /// that were reported by svn otherwise.
  int Get(
    wxString& contents,
    const wxString& fullpath = wxEmptyString);

  /// Gets info and shows results in a dialog.
  /// Returns true if dialog is accepted.
  bool Show(const wxString& fullpath = wxEmptyString);
private:
  const exSvnType m_Type;
  wxString m_Caption;
  wxString m_Command;
};
#endif

/*! \file */
// Clipboard handling.
/// Adds data to the clipboard.
bool exClipboardAdd(const wxString& text);
/// Gets data from the clipboard.
const wxString exClipboardGet();

/// Converts a colour (it's red, green, blue components) to a long.
long exColourToLong(const wxColour& c);

// Methods that do something with strings.
/// Adds an ellipses after text.
const wxString exEllipsed(
  const wxString& text,
  const wxString& control = wxEmptyString);
/// Returns the last specified number of characters from a string.
const wxString exGetEndOfWord(
  const wxString& text,
  size_t max_chars = 15);
/// Gets a line number from a string.
int exGetNumberOfLines(const wxString& text);
/// Gets a word from a string.
const wxString exGetWord(
  wxString& text,
  bool use_other_field_separators = false,
  bool use_path_separator = false);
int exGetLineNumberFromText(const wxString& text);
/// Returns a string without all white space in specified input.
const wxString exSkipWhiteSpace(
  const wxString& text,
  const wxString& replace_with = " ");
// This takes care of the translation.
const wxString exTranslate(const wxString& text, int pageNum, int numPages);

// Char methods.
/// Returns true if char is a brace open or close character.
bool exIsBrace(int c);
/// Returns true if char is a code word separator.
bool exIsCodewordSeparator(int c);
/// Returns true if char is alphanumeric or a _ sign.
bool exIsWordCharacter(wxChar c);

// Checks whether the file fullname matches a pattern (such as wildcard extensions separated by a ;).
/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool exMatchesOneOf(const wxFileName& filename, const wxString& patterns);

#if wxUSE_GUI

/// Shows the standard colour dialog with current background colour from specified window and
/// allows you to change that colour.
void exBackgroundColourDialog(wxWindow* parent, wxWindow* win);

// Combobox methods.
/// Adds entries from a combobox to a text string.
void exComboBoxFromString(
  wxComboBox* cb,
  const wxString& text,
  const wxChar field_separator = ',');
/// Adds entries from combobox to a text string.
bool exComboBoxToString(
  const wxComboBox* cb,
  wxString& text,
  const wxChar field_separator = ',',
  size_t max_items = 25);

/// Calls OpenFile for exFrame, if this is your top window.
void exOpenFile(const exFileName& filename, long open_flags = 0);

// Status bar.
#if wxUSE_STATUSBAR
/// Flags for exStatusText.
enum exStatusFlags
{
  STAT_DEFAULT  = 0x0000, ///< shows 'modified' and file 'fullname'
  STAT_SYNC     = 0x0001, ///< shows 'synchronized' instead of 'modified'
  STAT_FULLPATH = 0x0002, ///< shows file 'fullpath' instead of 'fullname'
};
/// Shows filename info on the statusbar.
void exStatusText(const exFileName& filename, long flags = STAT_DEFAULT);
#endif // wxUSE_STATUSBAR

#endif // wxUSE_GUI
#endif
