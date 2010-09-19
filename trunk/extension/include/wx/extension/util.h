/******************************************************************************\
* File:          util.h
* Purpose:       Include file for wxExtension utility functions
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXUTIL_H
#define _EXUTIL_H

#include <list>
#include <wx/combobox.h>
#include <wx/dir.h> // for wxDIR_DEFAULT
#include <wx/filedlg.h> // for wxFD_OPEN etc.
#include <wx/filename.h>
#include <wx/extension/filename.h>
#include <wx/extension/lexer.h>

class wxExFrame;
class wxXmlNode;

/*! \file */

/// Aligns text, if lexer is specified 
/// fills out over lexer comment lines.
/// If the lexer has no comment end character, fill out
/// with spaces is not done.
/// The header is used as a prefix for the line, directly 
/// followed by the lines, and if necessary on the next line
/// the header is repeated as a string of spaces.
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

/// Compares the files, using wxExecute on comparator set in the config.
bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2);

/// Returns first of a list of values from config key.
const wxString wxExConfigFirstOf(const wxString& key);

/// Adds an ellipses after text.
const wxString wxExEllipsed(
  const wxString& text,
  const wxString& control = wxEmptyString);

/// Finds other filenames from the one specified in the same dir structure.
/// Results are put on the list if not null, or in the filename if not null.
bool wxExFindOtherFileName(
  const wxFileName& filename,
  wxFileName* lastfile); // in case more files found, only most recent here

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

/// Gets field separator.
const wxUniChar wxExGetFieldSeparator();

/// Gets a line number from a string.
int wxExGetLineNumber(const wxString& text);

/// Gets the number of lines in a string.
int wxExGetNumberOfLines(const wxString& text);

/// Gets a word from a string.
const wxString wxExGetWord(
  wxString& text,
  bool use_other_field_separators = false,
  bool use_path_separator = false);

/// Loads entries from the config into a list with strings.
const std::list < wxString > wxExListFromConfig(
  const wxString& config);

/// Saves entries from a list with strings to the config.
void wxExListToConfig(
  const std::list < wxString > & l, 
  const wxString& config);

/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool wxExMatchesOneOf(const wxFileName& filename, const wxString& patterns);

/// Adds a caption.
const wxString wxExPrintCaption(const wxFileName& filename);

/// You can use macros in PrintFooter and in PrintHeader:
///   \@PAGENUM\@ is replaced by page number
///   \@PAGESCNT\@ is replaced by total number of pages
const wxString wxExPrintFooter();

/// Adds a header.
const wxString wxExPrintHeader(const wxFileName& filename);

/// Returns quotes around the text.
const wxString wxExQuoted(const wxString& text);

/// Returns a string without all white space in specified input.
const wxString wxExSkipWhiteSpace(const wxString& text);

/// This takes care of the translation.
const wxString wxExTranslate(const wxString& text, int pageNum, int numPages);

#if wxUSE_GUI

/// Adds entries to a combobox from a list with strings.
void wxExComboBoxFromList(
  wxComboBox* cb,
  const std::list < wxString > & text);

/// Adds entries from a combobox to a list with strings.
const std::list < wxString > wxExComboBoxToList(
  const wxComboBox* cb,
  size_t max_items = 25);

/// Opens files.
void wxExOpenFiles(wxExFrame* frame,
  const wxArrayString& files,
  long file_flags = 0,
  int dir_flags = wxDIR_DEFAULT);

/// Shows a dialog and opens selected files.
void wxExOpenFilesDialog(wxExFrame* frame,
  long style = wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR,
  const wxString& wildcards = wxFileSelectorDefaultWildcardStr,
  bool ask_for_continue = false,
  long file_flags = 0,
  int dir_flags = wxDIR_DEFAULT);

/// Executes VCS command id for specified path
/// and opens component if necessary.
void wxExVCSExecute(
  wxExFrame* frame, 
  int id,
  const wxExFileName& filename);
#endif // wxUSE_GUI

#endif
