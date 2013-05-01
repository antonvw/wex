////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Include file for wxExtension utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2013 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXUTIL_H
#define _EXUTIL_H

#include <list>
#include <vector>
#include <wx/combobox.h>
#include <wx/dir.h> // for wxDIR_DEFAULT
#include <wx/filedlg.h> // for wxFD_OPEN etc.
#include <wx/filename.h>
#include <wx/textctrl.h>
#include <wx/extension/lexer.h>

class wxExFileName;
class wxExFrame;
class wxExSTC;
class wxExVCSCommand;

/*! \file */

/// Aligns text.
const wxString wxExAlignText(
  /// lines to align
  const wxString& lines,
  /// The header is used as a prefix for the line, directly 
  /// followed by the lines, and if necessary on the next line
  /// the header is repeated as a string of spaces.
  const wxString& header,
  /// if fill out, then use space
  bool fill_out_with_space = false,
  /// fill out
  bool fill_out = false,
  /// if lexer is specified 
  /// fills out over lexer comment lines  
  /// If the lexer has no comment end character, fill out
  /// with spaces is not done.
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
/// The control, if present is postfixed, after a tab character (for accels).
const wxString wxExEllipsed(
  const wxString& text,
  const wxString& control = wxEmptyString);

/// Displays search result text in the statusbar.
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

/// Gets the icon index for this filename (uses the file extension to get it).
/// The return value is an index in wxTheFileIconsTable.
/// You can use this index as a bitmap using:
/// wxTheFileIconsTable->GetSmallImageList()->GetBitmap(wxExGetIconID(file))
int wxExGetIconID(const wxFileName& filename);

/// Gets the number of lines in a (trimmed) string.
/// If text is empty, 0 is returned, otherwise at least 1.
int wxExGetNumberOfLines(const wxString& text, bool trimmed = false);

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

/// Flags for wxExLogStatus.
enum wxExStatusFlags
{
  STAT_DEFAULT  = 0x0000, ///< shows 'modified' and file 'fullname'
  STAT_SYNC     = 0x0001, ///< shows 'synchronized' instead of 'modified'
  STAT_FULLPATH = 0x0002  ///< shows file 'fullpath' instead of 'fullname'
};

/// Logs filename info on the statusbar.
// Using type wxExStatusFlags instead of long gives compiler errors at
// invoking.
void wxExLogStatus(const wxExFileName& filename, long flags = STAT_DEFAULT);

/// Runs make on specified makefile.
/// Returns value from executing the make process.
long wxExMake(
  /// the makefile
  const wxFileName& makefile);

/// Regular expression match.
/// Returns number of submatches present in vector.
int wxExMatch(
  /// regular expression
  const wxString& regex,
  /// text to match
  const wxString& text, 
  /// vector is filled with submatches
  std::vector<wxString>& v);

/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool wxExMatchesOneOf(const wxFileName& filename, const wxString& patterns);

/// Parses properties node.
void wxExNodeProperties(
  const wxXmlNode* node,
  std::vector<wxExProperty>& properties);

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
const wxString wxExSkipWhiteSpace(
  const wxString& text,
  const wxString& replace_with = " ");

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
  
/// Gets a number from user, using hex display.
long wxExGetHexNumberFromUser(
  const wxString& message,
  const wxString& prompt,
  const wxString& caption,
  long value = 0,
  long min = 0,
  long max = 255,
  wxWindow *parent = NULL,
  const wxPoint& pos = wxDefaultPosition);

/// Opens files.
/// Opens all files specified by files.
void wxExOpenFiles(
  /// frame on which OpenFile for each file is called,
  /// and wxExDirOpenFile for each dir
  wxExFrame* frame,
  /// array with files
  const wxArrayString& files,
  /// flags to be used with OpenFile
  long file_flags = 0,
  /// flags to be used with wxExDirOpenFile
  int dir_flags = wxDIR_DEFAULT);

/// Shows a dialog and opens selected files
/// (calls wxExOpenFiles).
void wxExOpenFilesDialog(
  /// frame
  wxExFrame* frame,
  /// style for wxExFileDialog dialog
  long style = wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR,
  /// wilcards for wxExFileDialog dialog
  const wxString& wildcards = wxFileSelectorDefaultWildcardStr,
  /// flags to be used with wxExFileDialog
  bool ask_for_continue = false,
  /// flags to be used with OpenFile
  long file_flags = 0,
  /// flags to be used with wxExDirOpenFile
  int dir_flags = wxDIR_DEFAULT);

/// Sets a text ctrl value from a list of values.
void wxExSetTextCtrlValue(
  /// text ctrl
  wxTextCtrl* ctrl,
  /// UP or DOWN key
  int key,
  /// the list
  const std::list < wxString > & l,
  /// iterator on the list
  std::list < wxString >::const_iterator & it);

/// Use specified VCS command to set lexer on STC document.
void wxExVCSCommandOnSTC(
  /// VCS command, used to check for diff or open command
  const wxExVCSCommand& command, 
  /// lexer to be used
  const wxExLexer& lexer,
  /// stc on which lexer is set
  wxExSTC* stc);

/// Executes VCS command id for specified files
/// and opens component if necessary.
void wxExVCSExecute(
  /// frame on which OpenFile is called
  wxExFrame* frame, 
  /// VCS menu id to execute
  int id,
  /// files on which to operate
  const wxArrayString& files);
#endif // wxUSE_GUI

#endif
