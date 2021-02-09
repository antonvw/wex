////////////////////////////////////////////////////////////////////////////////
// Name:      core.h
// Purpose:   Include file for wex core utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <set>
#include <vector>
#include <wex/style.h>

class wxArrayString;
class wxWindow;

namespace pugi
{
  class xml_node;
  struct xml_parse_result;
}; // namespace pugi

namespace wex
{
  class path;
  class property;

  namespace core
  {
    class stc;
  };

  /*! \file */

  /// Returns string after first or last occurrence of c
  /// Returns the whole string if c is not found.
  const std::string after(const std::string& text, char c, bool first = true);

  /// Tries to auto_complete filename,
  /// the result is stored in the tuple.
  std::tuple<
    /// true if a match was found
    bool,
    /// expansion of text to matching filename
    /// (if only 1 match exists)
    /// or common part of matching filenames
    const std::string,
    /// vector containing completed file name(s)
    const std::vector<std::string>>
  auto_complete_filename(
    /// text containing start of a filename
    const std::string& text);

  /// Tries to auto_complete text from a vector of strings,
  /// result stored in the string.
  /// Returns true if a unique match was found.
  bool auto_complete_text(
    /// text to be completed
    const std::string& text,
    /// vector with completed text
    const std::vector<std::string>& v,
    /// expansion of text to one of the strings from the vector
    std::string& s);

  /// Returns string before first or last occurrence of c
  /// Returns the whole string if c is not found.
  const std::string before(const std::string& text, char c, bool first = true);

  /// Launch default browser.
  /// Returns false if no browser configured.
  bool browser(const std::string& url);

  /// Browse and search for text.
  /// Returns false if search engine is empty.
  bool browser_search(const std::string& text);

  /// Adds data to the clipboard.
  bool clipboard_add(const std::string& text);

  /// Returns data from the clipboard.
  const std::string clipboard_get();

  /// Adds an ellipses after text.
  /// The control, if present is postfixed, after a tab character (for accels).
  const std::string ellipsed(
    const std::string& text,
    const std::string& control = std::string(),
    bool               ellipse = true);

  enum
  {
    FIRST_OF_AFTER_FROM_BEGIN = 0, ///< substring after match, from begin
    FIRST_OF_BEFORE           = 1, ///< substring before match
    FIRST_OF_FROM_END         = 2, ///< substring from end
  };

  typedef std::bitset<3> first_of_t;

  /// Returns substring after (or before) first occurrence of one of specified
  /// chars.
  const std::string first_of(
    /// text to be searched
    const std::string& text,
    /// chars to be found
    const std::string& chars,
    /// start pos (from start or end of text, depending on flags)
    const size_t start_pos = 0,
    /// start searching at begin, or at end
    first_of_t flags = first_of_t().set(FIRST_OF_AFTER_FROM_BEGIN));

  /// If text length exceeds max_chars,
  /// returns an ellipse prefix followed by the last max_chars from the text,
  /// otherwise just returns the text.
  const std::string
  get_endoftext(const std::string& text, size_t max_chars = 15);

  /// Returns a search result, that might be shown in the statusbar.
  const std::string
  get_find_result(const std::string& find_text, bool find_next, bool recursive);

  /// Returns the icon index for this filename (uses the file extension to get
  /// it). The return value is an index in wxTheFileIconsTable. You can use this
  /// index as a bitmap using:
  /// wxTheFileIconsTable->GetSmallImageList()->GetBitmap(get_iconid(file))
  int get_iconid(const path& filename);

  /// Returns the number of lines in a (trimmed) string.
  /// If text is empty, 0 is returned, otherwise at least 1.
  int get_number_of_lines(const std::string& text, bool trimmed = false);

  /// Returns string from set.
  const std::string get_string_set(
    const std::set<std::string>& kset,
    size_t                       min_size = 0,
    const std::string&           prefix   = std::string());

  /// Returns a word from a string.
  const std::string get_word(std::string& text);

  /// Returns true if char is a brace open or close character.
  bool is_brace(int c);

  /// Returns true if char is a code word separator.
  bool is_codeword_separator(int c);

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
    const pugi::xml_node*  node,
    std::vector<property>& properties);

  /// Parses style node.
  void node_styles(
    const pugi::xml_node* node,
    const std::string&    lexer,
    std::vector<style>&   styles);

  /// Returns whether there is one letter after.
  bool one_letter_after(const std::string& text, const std::string& letter);

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

  enum
  {
    STRING_SORT_DESCENDING = 0, ///< sort descending order
    STRING_SORT_UNIQUE     = 1, ///< flag to remove doubles
  };

  typedef std::bitset<2> string_sort_t;

  /// Presents a dialog to choose one string out of an array.
  bool single_choice_dialog(
    wxWindow*            parent,
    const std::string&   title,
    const wxArrayString& s,
    std::string&         selection);

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
    core::stc* stc,
    /// sort type
    string_sort_t sort_t = 0,
    /// position of the first character to be replaced
    size_t pos = 0,
    /// number of characters to replace
    /// string::npos indicates all characters until eol
    size_t len = std::string::npos);

  /// This takes care of the translation.
  const std::string
  translate(const std::string& text, int pageNum, int numPages);
}; // namespace wex
