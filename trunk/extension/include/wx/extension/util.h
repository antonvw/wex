/******************************************************************************\
* File:          util.h
* Purpose:       Include file for wxextension utility functions
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXUTIL_H
#define _EXUTIL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/filename.h>

/*! \file */

/// Adds data to the clipboard.
bool exClipboardAdd(const wxString& text);

/// Gets data from the clipboard.
const wxString exClipboardGet();

/// Converts a colour (it's red, green, blue components) to a long.
long exColourToLong(const wxColour& c);

/// Adds an ellipses after text.
const wxString exEllipsed(
  const wxString& text,
  const wxString& control = wxEmptyString);

/// Returns the last specified number of characters from a string.
const wxString exGetEndOfWord(
  const wxString& text,
  size_t max_chars = 15);

/// Gets the number of lines in a string.
int exGetNumberOfLines(const wxString& text);

/// Gets a line number from a string.
int exGetLineNumberFromText(const wxString& text);

/// Returns the filename of the logfile.
const wxFileName exLogfileName();

/// Logs text with a timestamp at the end of the file.
void exLog(const wxString& text, const wxFileName& filename = exLogfileName());

/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool exMatchesOneOf(const wxFileName& filename, const wxString& patterns);

/// Returns a string without all white space in specified input.
const wxString exSkipWhiteSpace(
  const wxString& text,
  const wxString& replace_with = " ");

/// This takes care of the translation.
const wxString exTranslate(const wxString& text, int pageNum, int numPages);

#if wxUSE_GUI
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
#endif // wxUSE_GUI

#endif
