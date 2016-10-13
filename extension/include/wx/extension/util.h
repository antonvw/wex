////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Include file for wxExtension utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wx/combobox.h>
#include <wx/dir.h> // for wxDIR_DEFAULT
#include <wx/filedlg.h> // for wxFD_OPEN etc.

class wxArrayString;
class wxFileName;
class wxXmlNode;

class wxExEx;
class wxExFileName;
class wxExFrame;
class wxExLexer;
class wxExProperty;
class wxExSTC;
class wxExStyle;
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
  bool fill_out_with_space,
  /// fill out
  bool fill_out,
  /// if lexer is specified 
  /// fills out over lexer comment lines  
  /// If the lexer has no comment end character, fill out
  /// with spaces is not done.
  const wxExLexer& lexer);

/// Tries to autocomplete text from a vector of strings,
/// result stored in the string.
/// Returns true if a unique match was found.
bool wxExAutoComplete(
  /// text to be completed
  const std::string& text, 
  /// vector with completed text
  const std::vector<std::string> & v,
  /// expansion of text to one of the strings from the vector
  std::string& s);

/// Tries to autocomplete filename,
/// result stored in the vector.
bool wxExAutoCompleteFileName(
  /// text containing start of a filename
  const std::string& text, 
  /// vector containing completed file name(s)
  /// v[0] is expansion of text to matching filename
  /// (if only 1 match exists)
  /// or common part of matching filenames
  /// other elements are all matching file names
  /// Returns true if a match was found 
  /// (and v contains at least 2 elements).
  std::vector<std::string> & v);
  
/// Adds data to the clipboard.
bool wxExClipboardAdd(const wxString& text);

/// Returns data from the clipboard.
const std::string wxExClipboardGet();

#if wxUSE_GUI
/// Adds entries to a combobox from a container.
template <typename T> 
void wxExComboBoxAs(wxComboBox* cb, const T& t)
{
  if (!t.empty())
  {
    cb->Clear();

    wxArrayString as;
    as.resize(t.size());
    std::copy(t.begin(), t.end(), as.begin());
    
    cb->Append(as);
    cb->SetValue(cb->GetString(0));
  }
}

/// Adds entries to a combobox from a list with strings.
void wxExComboBoxFromList(
  wxComboBox* cb,
  const std::list < wxString > & text);
#endif
  
/// Compares the files, using wxExecute on comparator set in the config.
bool wxExCompareFile(const wxFileName& file1, const wxFileName& file2);

/// Returns the config dir for user data files.
const wxString wxExConfigDir();

/// Returns first of a list of values from config key.
const wxString wxExConfigFirstOf(const wxString& key);

/// Sets first of a list of values in config key.
/// And returns the value.
const wxString wxExConfigFirstOfWrite(const wxString& key, const wxString& value);

/// Adds an ellipses after text.
/// The control, if present is postfixed, after a tab character (for accels).
const wxString wxExEllipsed(
  const wxString& text,
  const wxString& control = wxEmptyString,
  bool ellipse = true);

/// If text length exceeds max_chars,
/// returns an ellipse prefix followed by the last max_chars from the text,
/// otherwise just returns the text.
const wxString wxExGetEndOfText(
  const wxString& text,
  size_t max_chars = 15);

/// Returns field separator.
const wxUniChar wxExGetFieldSeparator();

/// Returns a search result, that might be shown in the statusbar.
const wxString wxExGetFindResult(
  const wxString& find_text, 
  bool find_next, 
  bool recursive);

/// Returns the icon index for this filename (uses the file extension to get it).
/// The return value is an index in wxTheFileIconsTable.
/// You can use this index as a bitmap using:
/// wxTheFileIconsTable->GetSmallImageList()->GetBitmap(wxExGetIconID(file))
int wxExGetIconID(const wxFileName& filename);

/// Returns the number of lines in a (trimmed) string.
/// If text is empty, 0 is returned, otherwise at least 1.
int wxExGetNumberOfLines(const wxString& text, bool trimmed = false);

/// Returns a word from a string.
const wxString wxExGetWord(
  wxString& text,
  bool use_other_field_separators = false,
  bool use_path_separator = false);

/// Returns true if char is a brace open or close character.
bool wxExIsBrace(int c);
         
/// Returns true if char is a code word separator.
bool wxExIsCodewordSeparator(int c);

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

/// Expands all markers and registers in command.
/// Returns false if a marker could not be found.
bool wxExMarkerAndRegisterExpansion(wxExEx* ex, wxString& command);

/// Regular expression match.
/// Returns number of submatches present in vector.
int wxExMatch(
  /// regular expression
  const std::string& regex,
  /// text to match
  const std::string& text, 
  /// vector is filled with submatches
  std::vector<wxString>& v);

/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool wxExMatchesOneOf(const wxFileName& filename, const wxString& patterns);

/// Parses properties node.
void wxExNodeProperties(
  const wxXmlNode* node,
  std::vector<wxExProperty>& properties);
  
/// Parses style node.
void wxExNodeStyles(
  const wxXmlNode* node,
  const wxString& lexer,
  std::vector<wxExStyle>& styles);

#if wxUSE_GUI
/// Opens all files specified by files.
/// Returns number of files opened.
int wxExOpenFiles(
  /// frame on which OpenFile for each file is called,
  /// and wxExDirOpenFile for each dir
  wxExFrame* frame,
  /// array with files
  const std::vector< wxString > & files,
  /// flags to be used with OpenFile
  long file_flags = 0,
  /// flags to be used with wxExDirOpenFile
  int dir_flags = wxDIR_DEFAULT,
  /// initial vi command
  const wxString& command = wxEmptyString);
#endif

/// Shows a dialog and opens selected files
#if wxUSE_GUI
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
#endif

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

/// Executes all process between backquotes in command, 
/// and changes command with replaced match with output from process.
/// Returns false if process could not be executed.
bool wxExShellExpansion(wxString& command);
  
enum
{
  STRING_SORT_ASCENDING  = 0x000, ///< default, keep doubles
  STRING_SORT_DESCENDING = 0x001, ///< sort descending order
  STRING_SORT_UNIQUE     = 0x010, ///< flag to remove doubles
};

/// Sorts specified text, returns string with sorted text.
const wxString wxExSort(
  /// text to sort
  const wxString& input, 
  /// sort type
  size_t sort_type,
  /// position of the first character to be replaced
  size_t pos, 
  /// eol to split lines
  const wxString& eol,
  /// number of characters to replace
  /// string::npos indicates all characters until eol
  size_t len = std::string::npos);
  
#if wxUSE_GUI
/// Sorts specified component, returns true if sorted ok.
bool wxExSortSelection(
  /// Component with selected text to be sorted
  wxExSTC* stc,
  /// sort type
  size_t sort_type = STRING_SORT_ASCENDING,
  /// position of the first character to be replaced
  size_t pos = 0,
  /// number of characters to replace
  /// string::npos indicates all characters until eol
  size_t len = std::string::npos);
#endif

/// Returns a string without all white space in specified input.
const std::string wxExSkipWhiteSpace(
  const std::string& text,
  const std::string& replace_with = " ");

/// This takes care of the translation.
const wxString wxExTranslate(const wxString& text, int pageNum, int numPages);

#if wxUSE_GUI
/// Use specified VCS command to set lexer on STC document.
void wxExVCSCommandOnSTC(
  /// VCS command, used to check for diff or open command
  const wxExVCSCommand& command, 
  /// lexer to be used
  const wxExLexer& lexer,
  /// stc on which lexer is set
  wxExSTC* stc);
#endif

#if wxUSE_GUI
/// Executes VCS command id for specified files
/// and opens component if necessary.
void wxExVCSExecute(
  /// frame on which OpenFile is called
  wxExFrame* frame, 
  /// VCS menu id to execute
  int id,
  /// files on which to operate
  const std::vector< wxString > & files);
#endif
