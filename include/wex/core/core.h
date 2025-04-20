////////////////////////////////////////////////////////////////////////////////
// Name:      core.h
// Purpose:   Include file for wex core utility functions
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <set>
#include <string>
#include <vector>

namespace wex
{
/*! \file */

/// Tries to auto_complete text from a vector of strings,
/// result stored in the string.
/// Returns true if a unique match was found.
bool auto_complete_text(
  /// text to be completed
  const std::string& text,
  /// vector with completed text
  const std::vector<std::string>& v,
  /// expansion of text to the last match from the vector
  std::string& s);

/// Bells if enabled in config.
bool bell();

/// Launch default browser.
/// Returns false if no browser configured or url could not be opened.
bool browser(const std::string& url);

/// Browse and search for text.
/// Returns false if search engine is empty or browse returns false.
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

/// Returns string after first occurrence of sequence
/// Returns the whole string if seq is not found.
const std::string
find_after(const std::string& text, const std::string& sequence);

/// Returns string before first occurrence of sequence
/// Returns the whole string if seq is not found.
const std::string
find_before(const std::string& text, const std::string& sequence);

/// If text length exceeds max_chars,
/// returns an ellipse prefix followed by the last max_chars from the text,
/// otherwise just returns the text.
const std::string find_tail(const std::string& text, size_t max_chars = 15);

/// Returns the number of lines in a (trimmed) string.
/// If text is empty, 0 is returned, otherwise at least 1.
int get_number_of_lines(const std::string& text, bool trimmed = false);

/// Returns string from set.
const std::string get_string_set(
  const std::set<std::string>& kset,
  size_t                       min_size = 0,
  const std::string&           prefix   = std::string());

/// Returns 0 if both texts are equal, ignoring case.
int icompare(const std::string& text1, const std::string& text2);

/// Returns true if text contains sequence, ignoring case.
bool icontains(const std::string& text, const std::string& sequence);

/// Returns true if char is a brace open or close character.
bool is_brace(int c);

/// Returns true if char is a code word separator.
bool is_codeword_separator(int c);

/// Returns true if filename (fullname) matches one of the
/// fields in the specified pattern
bool matches_one_of(
  /// the fullname
  const std::string& fullname,
  /// the pattern to match, fields separated by ; sign
  const std::string& patterns,
  /// default the pattern is not a regex, but you can change it
  bool is_regex = false);

/// Returns delimiter around the text.
const std::string quoted(const std::string& text, char delim = '\'');

/// Returns double quotes around the text if text contains c,
/// otherwise returns text.
const std::string quoted_find(const std::string& text, char c = ' ');

/// Returns string after last occurrence of sequence
/// Returns the whole string if seq is not found.
const std::string
rfind_after(const std::string& text, const std::string& sequence);

/// Returns string before last occurrence of sequence
/// Returns the whole string if seq is not found.
const std::string
rfind_before(const std::string& text, const std::string& sequence);
} // namespace wex
