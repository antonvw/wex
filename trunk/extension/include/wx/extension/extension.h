/******************************************************************************\
* File:          extension.h
* Purpose:       Include file for most wxWidgets extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _WXEXTENSION_H
#define _WXEXTENSION_H

#include <wx/extension/version.h>
#include <wx/extension/base.h>
#include <wx/extension/app.h>
#include <wx/extension/file.h>

// Log methods.
/// Returns the filename of the logfile.
const exFileName exLogfileName();
/// Logs text with a timestamp at the end of the file.
void exLog(const wxString& text, const exFileName& filename = exLogfileName());

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
int exGetLineNumberFromText(const wxString& text);
/// Returns a string without all white space in specified input.
const wxString exSkipWhiteSpace(
  const wxString& text,
  const wxString& replace_with = " ");
// This takes care of the translation.
const wxString exTranslate(const wxString& text, int pageNum, int numPages);

// Char methods.
// Checks whether the file fullname matches a pattern (such as wildcard extensions separated by a ;).
/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool exMatchesOneOf(const wxFileName& filename, const wxString& patterns);

#if wxUSE_GUI

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

#endif // wxUSE_GUI
#endif
