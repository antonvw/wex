////////////////////////////////////////////////////////////////////////////////
// Name:      util.h
// Purpose:   Include file for wex utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <vector>
#include <wx/combobox.h>
#include <wx/filedlg.h> // for wxFD_OPEN etc.
#include <wex/dir.h>
#include <wex/stc-data.h>

class wxArrayString;
namespace pugi
{
  class xml_node;
  struct xml_parse_result;
};

namespace wex
{
  class ex;
  class frame;
  class lexer;
  class path;
  class property;
  class stc;
  class style;
  class vcs_command;

  /*! \file */

  /// Returns string after first or last occurrence of c
  /// Returns the whole string if c is not found.
  const std::string after(
    const std::string& text, char c, bool first = true);

  /// Aligns text.
  const std::string align_text(
    /// lines to align
    const std::string_view& lines,
    /// The header is used as a prefix for the line, directly 
    /// followed by the lines, and if necessary on the next line
    /// the header is repeated as a string of spaces.
    const std::string_view& header,
    /// if fill out, then use space
    bool fill_out_with_space,
    /// fill out
    bool fill_out,
    /// if lexer is specified 
    /// fills out over lexer comment lines  
    /// If the lexer has no comment end character, fill out
    /// with spaces is not done.
    const lexer& lexer);

  /// Tries to autocomplete filename,
  /// the result is stored in the tuple.
  std::tuple<
    /// true if a match was found 
    bool, 
    /// expansion of text to matching filename
    /// (if only 1 match exists)
    /// or common part of matching filenames
    const std::string, 
    /// vector containing completed file name(s)
    const std::vector<std::string>> autocomplete_filename(
    /// text containing start of a filename
    const std::string& text); 
    
  /// Tries to autocomplete text from a vector of strings,
  /// result stored in the string.
  /// Returns true if a unique match was found.
  bool autocomplete_text(
    /// text to be completed
    const std::string& text, 
    /// vector with completed text
    const std::vector<std::string> & v,
    /// expansion of text to one of the strings from the vector
    std::string& s);

  /// Returns string before first or last occurrence of c
  /// Returns the whole string if c is not found.
  const std::string before(
    const std::string& text, char c, bool first = true);

  /// Launch default browser and search for text.
  /// Returns false if search engine is empty.
  bool browser_search(const std::string& text);

  /// Adds data to the clipboard.
  bool clipboard_add(const std::string& text);

  /// Returns data from the clipboard.
  const std::string clipboard_get();

  /// Adds entries to a combobox from a container.
  template <typename T> 
  void combobox_as(wxComboBox* cb, const T& t)
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
  void combobox_from_list(
    wxComboBox* cb,
    const std::list < std::string > & text);
    
  /// Compares the files, using comparator set in the config.
  bool comparefile(const path& file1, const path& file2);

  /// Adds an ellipses after text.
  /// The control, if present is postfixed, after a tab character (for accels).
  const std::string ellipsed(
    const std::string& text,
    const std::string& control = std::string(),
    bool ellipse = true);

  enum
  {
    FIRST_OF_AFTER_FROM_BEGIN = 0, ///< substring after match, from begin
    FIRST_OF_BEFORE           = 1, ///< substring before match
    FIRST_OF_FROM_END         = 2, ///< substring from end
  };

  typedef std::bitset<3> firstof_t;
  
  /// Returns substring after (or before) first occurrence of one of specified chars.
  const std::string firstof(
    /// text to be searched
    const std::string& text, 
    /// chars to be found
    const std::string& chars,
    /// start pos (from start or end of text, depending on flags)
    const size_t start_pos = 0,
    /// start searching at begin, or at end
    firstof_t flags = firstof_t().set(FIRST_OF_AFTER_FROM_BEGIN));

  /// If text length exceeds max_chars,
  /// returns an ellipse prefix followed by the last max_chars from the text,
  /// otherwise just returns the text.
  const std::string get_endoftext(
    const std::string& text,
    size_t max_chars = 15);

  /// Returns field separator.
  const char get_field_separator();

  /// Returns a search result, that might be shown in the statusbar.
  const std::string get_find_result(
    const std::string& find_text, 
    bool find_next, 
    bool recursive);

  /// Returns the icon index for this filename (uses the file extension to get it).
  /// The return value is an index in wxTheFileIconsTable.
  /// You can use this index as a bitmap using:
  /// wxTheFileIconsTable->GetSmallImageList()->GetBitmap(get_iconid(file))
  int get_iconid(const path& filename);

  /// Returns the number of lines in a (trimmed) string.
  /// If text is empty, 0 is returned, otherwise at least 1.
  int get_number_of_lines(const std::string& text, bool trimmed = false);

  /// Returns string from set.
  const std::string get_string_set(
    const std::set<std::string>& kset, 
    size_t min_size = 0,
    const std::string& prefix = std::string());

  /// Converts time string into time_t.
  /// The result is stored in the tuple.
  std::tuple <
    /// true if text could be converted into time_t
    bool, 
    /// the converted time
    time_t> get_time(
      /// text to be converted
      const std::string& text, 
      /// format to be used for cnoversion
      const std::string& format = file_stat::MOD_TIME_FORMAT);

  /// Returns a word from a string.
  const std::string get_word(
    std::string& text,
    bool use_other_field_separators = false,
    bool use_path_separator = false);

  /// Returns true if char is a brace open or close character.
  bool is_brace(int c);
           
  /// Returns true if char is a code word separator.
  bool is_codeword_separator(int c);

  /// Flags for log_status.
  enum
  {
    STAT_SYNC     = 0, ///< shows 'synchronized' instead of 'modified'
    STAT_FULLPATH = 1  ///< shows file 'fullpath' instead of 'fullname'
  };

  typedef std::bitset<2> status_t;
  
  /// Logs text.
  void log_status(const char* text);

  /// Logs text.
  void log_status(const std::string& text);

  /// Logs path info on the statusbar.
  void log_status(const path& filename, status_t flags = 0);

  /// Runs make on specified makefile.
  /// Returns value from executing the make process.
  long make(
    /// the makefile
    const path& makefile);

  /// Expands all markers and registers in command.
  /// Returns false if a marker could not be found.
  bool marker_and_register_expansion(ex* ex, std::string& command);

  /// Regular expression match.
  /// Returns:
  /// - -1 if text does not match or there is an error
  /// - 0 if text matches, but no submatches present, v is untouched
  /// - submatches, it fills v with the submatches
  int match(
    /// regular expression
    const std::string& regex,
    /// text to match
    const std::string& text, 
    /// vector is filled with submatches
    std::vector<std::string>& v);

  /// Returns true if filename (fullname) matches one of the
  /// fields in specified pattern (fields separated by ; sign).
  bool matches_one_of(const std::string& fullname, const std::string& patterns);

  /// Parses properties node.
  void node_properties(
    const pugi::xml_node* node,
    std::vector<property>& properties);
    
  /// Parses style node.
  void node_styles(
    const pugi::xml_node* node,
    const std::string& lexer,
    std::vector<style>& styles);

  /// Returns whether there is one letter after.
  bool one_letter_after(const std::string& text, const std::string& letter);

  /// Opens all files specified by files.
  /// Returns number of files opened.
  int open_files(
    /// frame on which open_file for each file is called,
    /// and open_file_dir for each dir
    frame* frame,
    /// array with files
    const std::vector< path > & files,
    /// data to be used with open_file
    const stc_data& data = stc_data(),
    /// flags to be used with open_file_dir
    dir::type_t type = dir::type_t().set());

  /// Shows a dialog and opens selected files
  /// (calls open_files).
  void open_files_dialog(
    /// frame
    frame* frame,
    /// style for file_dialog dialog
    long style = wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR,
    /// wilcards for file_dialog dialog
    const std::string& wildcards = wxFileSelectorDefaultWildcardStr,
    /// flags to be used with file_dialog
    bool ask_for_continue = false,
    /// data to be used with open_file
    const stc_data& data = stc_data(),
    /// flags to be used with open_file_dir
    dir::type_t type = dir::type_t().set());

  /// Adds a caption.
  const std::string print_caption(const path& filename);

  /// You can use macros in PrintFooter and in PrintHeader:
  ///   \@PAGENUM\@ is replaced by page number
  ///   \@PAGESCNT\@ is replaced by total number of pages
  const std::string print_footer();

  /// Adds a header.
  const std::string print_header(const path& filename);

  /// Returns quotes around the text.
  const std::string quoted(const std::string& text);

  /// Returns true if a register is specified after letter in text.
  bool regafter(const std::string& text, const std::string& letter);

  /// Replaces all substrings in text with replace.
  /// Returns number of replacements.
  int replace_all(
    /// text to be replaced
    std::string& text, 
    /// text to replace (no regex)
    const std::string& search,
    /// replacement
    const std::string& replace,
    /// if not nullptr, position of first match in text
    int* match_pos = nullptr);

  /// Executes all process between backquotes in command, 
  /// and changes command with replaced match with output from process.
  /// Returns false if process could not be executed.
  bool shell_expansion(std::string& command);

  enum
  {
    SKIP_LEFT  = 0, ///< skip space at left
    SKIP_MID   = 1, ///< skip space at mid
    SKIP_RIGHT = 2, ///< skip space at right
  };
    
  typedef std::bitset<3> skip_t;
  
  /// Returns a string without all white space in specified input.
  const std::string skip_white_space(
    /// text with white space to be skipped
    const std::string& text,
    /// kind of skip
    skip_t type_t = skip_t().set(SKIP_LEFT).set(SKIP_RIGHT),
    /// replace with (only for SKIP_MID)
    const std::string& replace_with = " ");

  enum
  {
    STRING_SORT_DESCENDING = 0, ///< sort descending order
    STRING_SORT_UNIQUE     = 1, ///< flag to remove doubles
  };

  typedef std::bitset<2> string_sort_t;
  
  /// Sorts specified text, returns string with sorted text.
  const std::string sort(
    /// text to sort
    const std::string& input, 
    /// sort type
    string_sort_t sort_t,
    /// position of the first character to be replaced
    size_t pos, 
    /// eol to split lines
    const std::string& eol,
    /// number of characters to replace
    /// string::npos indicates all characters until eol
    size_t len = std::string::npos);
    
  /// Sorts specified component, returns true if sorted ok.
  bool sort_selection(
    /// Component with selected text to be sorted
    stc* stc,
    /// sort type
    string_sort_t sort_t = 0,
    /// position of the first character to be replaced
    size_t pos = 0,
    /// number of characters to replace
    /// string::npos indicates all characters until eol
    size_t len = std::string::npos);

  /// This takes care of the translation.
  const std::string translate(const std::string& text, int pageNum, int numPages);

  /// Use specified VCS command to set lexer on STC document.
  void vcs_command_stc(
    /// VCS command, used to check for diff or open command
    const vcs_command& command, 
    /// lexer to be used
    const lexer& lexer,
    /// stc on which lexer is set
    stc* stc);

  /// Executes VCS command id for specified files
  /// and opens component if necessary.
  void vcs_execute(
    /// frame on which open_file is called
    frame* frame, 
    /// VCS menu id to execute
    int id,
    /// files on which to operate
    const std::vector< path > & files);

  /// Shows xml error.
  void xml_error(
    /// xml filename that has error
    const path& filename, 
    /// result of parsing describing the error
    const pugi::xml_parse_result* result,
    /// stc component containing the filename
    stc* stc = nullptr);
};
