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
#include <wx/extension/lexer.h>

/*! \file */

/// Aligns text, if lexer is speecified 
/// fills out over lexer comment lines.
const wxString wxExAlignText(
  const wxString& lines,
  const wxString& header,
  bool fill_out_with_space = false,
  bool fill_out = false,
  const wxExLexer& lexer = wxExLexer());

/// Adds data to the clipboard.
bool wxExClipboardAdd(const wxString& text);

/// Gets data from the clipboard.
const wxString wxExClipboardGet();

/// Converts a colour (it's red, green, blue components) to a long.
long wxExColourToLong(const wxColour& c);

/// Adds an ellipses after text.
const wxString wxExEllipsed(
  const wxString& text,
  const wxString& control = wxEmptyString);

/// Shows searching for in the statusbar.
void wxExFindResult(
  const wxString& find_text, 
  bool find_next, 
  bool recursive);

/// If text length exceeds max_chars,
/// returns an ellipse prefix followed by the last max_chars from the text,
/// otherwise just returns the text.
const wxString wxExGetEndOfText(
  const wxString& text,
  size_t max_chars = 15);

/// Gets the number of lines in a string.
int wxExGetNumberOfLines(const wxString& text);

/// Gets a line number from a string.
int wxExGetLineNumberFromText(const wxString& text);

/// Gets a word from a string.
const wxString wxExGetWord(
  wxString& text,
  bool use_other_field_separators = false,
  bool use_path_separator = false);

/// Returns the filename of the logfile.
const wxFileName wxExLogfileName();

/// Logs text with a timestamp at the end of the file.
/// Returns true if text was written succesfully.
bool wxExLog(const wxString& text, const wxFileName& filename = wxExLogfileName());

/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool wxExMatchesOneOf(const wxFileName& filename, const wxString& patterns);

/// Returns quotes around the text.
const wxString wxExQuoted(const wxString& text);

/// Adds a caption.
const wxString wxExPrintCaption(const wxFileName& filename);

/// You can use macros in PrintFooter and in PrintHeader:
///   \@PAGENUM\@ is replaced by page number
///   \@PAGESCNT\@ is replaced by total number of pages
const wxString wxExPrintFooter();

/// Adds a header.
const wxString wxExPrintHeader(const wxFileName& filename);

/// Returns a string without all white space in specified input.
const wxString wxExSkipWhiteSpace(
  const wxString& text,
  const wxString& replace_with = " ");

/// This takes care of the translation.
const wxString wxExTranslate(const wxString& text, int pageNum, int numPages);

#if wxUSE_GUI
/// Adds entries from a combobox to a text string.
void wxExComboBoxFromString(
  wxComboBox* cb,
  const wxString& text,
  const wxChar field_separator = ',');

/// Adds entries from combobox to a text string.
bool wxExComboBoxToString(
  const wxComboBox* cb,
  wxString& text,
  const wxChar field_separator = ',',
  size_t max_items = 25);

/// Calls OpenFile for wxExFrame, if this is your top window.
bool wxExOpenFile(const wxFileName& filename, long open_flags = 0);
#endif // wxUSE_GUI

#endif
