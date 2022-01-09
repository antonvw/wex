////////////////////////////////////////////////////////////////////////////////
// Name:      core.h
// Purpose:   Include file for wex core utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <bitset>
#include <set>
#include <vector>

class wxWindow;

namespace wex
{
class path;

/*! \file */

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

/// Returns string after first or last occurrence of sequence
/// Returns the whole string if c is not found.
const std::string find_after(
  const std::string& text,
  const std::string& sequence,
  bool               first = true);

/// Returns string before first or last occurrence of sequence
/// Returns the whole string if c is not found.
const std::string find_before(
  const std::string& text,
  const std::string& sequence,
  bool               first = true);

/// If text length exceeds max_chars,
/// returns an ellipse prefix followed by the last max_chars from the text,
/// otherwise just returns the text.
const std::string find_tail(const std::string& text, size_t max_chars = 15);

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

/// Returns true if filename (fullname) matches one of the
/// fields in specified pattern (fields separated by ; sign).
bool matches_one_of(const std::string& fullname, const std::string& patterns);

/// Returns quotes around the text.
const std::string quoted(const std::string& text);

/// Returns true if a register is specified after letter in text.
bool regafter(const std::string& text, const std::string& letter);

/// Presents a dialog to choose one string out of an array.
bool single_choice_dialog(
  wxWindow*                       parent,
  const std::string&              title,
  const std::vector<std::string>& v,
  std::string&                    selection);

/// This takes care of the translation.
const std::string translate(const std::string& text, int pageNum, int numPages);
} // namespace wex
