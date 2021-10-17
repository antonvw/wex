////////////////////////////////////////////////////////////////////////////////
// Name:      frd.h
// Purpose:   Declaration of wex::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <regex>
#include <string>

class wxFindReplaceData;

namespace wex::factory
{
/// Offers a class to hold data for find replace functionality.
class find_replace_data
{
public:
  /// Static interface.

  /// Returns text.
  static const auto& text_find() { return m_text_find; }

  /// Returns text.
  static const auto& text_match_case() { return m_text_match_case; }

  /// Returns text.
  static const auto& text_match_word() { return m_text_match_word; }

  /// Returns text.
  static const auto& text_regex() { return m_text_regex; }

  /// Returns text.
  static const auto& text_replace_with() { return m_text_replace_with; }

  /// Returns text.
  static const auto& text_search_down() { return m_text_search_down; }

  /// Other methods.

  /// Default constructor.
  find_replace_data();

  /// Destructor, writes data to config.
  ~find_replace_data();

  /// Virtual interface.

  /// Sets the find string.
  /// If use_regex also sets the regular expression.
  /// This string is used for tool find in files and replace in files.
  /// Also moves the find string to the beginning of the find
  /// strings list.
  virtual void set_find_string(const std::string& value);

  /// Sets the replace string.
  /// Also moves the replace string to the beginning of the replace
  /// strings list.
  virtual void set_replace_string(const std::string& value);

  /// Other methods.

  /// Access to data.
  wxFindReplaceData* data();

  /// Returns the find string.
  const std::string get_find_string() const;

  /// Returns the replace string.
  const std::string get_replace_string() const;

  /// Returns true if find text is used as a regular expression.
  bool is_regex() const { return m_use_regex; }

  /// Returns true if the flags have match case set.
  bool match_case() const;

  /// Returns true if the flags have whole word set.
  bool match_word() const;

  /// Replaces all occurrences of the find string as regular expression
  /// in text by the replace string.
  /// Returns number of replacements done in text.
  int regex_replace(std::string& text) const;

  /// Finds the find string in text.
  /// Returns -1 if find string as regular expression does not match text,
  /// otherwise the start pos of the match.
  int regex_search(const std::string& text) const;

  /// Returns true if the flags have search down set.
  bool search_down() const;

  /// Sets flags for match case.
  void set_match_case(bool value);

  /// Sets flags for match word.
  void set_match_word(bool value);

  /// Sets using regular expression for find text.
  /// If get_find_string does not contain a valid regular expression
  /// the use member is not set.
  void set_regex(bool value);

  /// Sets flags for search down.
  void set_search_down(bool value);

  /// Returns wx frd.
  auto* wx() const { return m_frd; }

private:
  // initializing them using _(), does not work,
  // then they are not translated
  static inline std::string m_text_find, m_text_match_case, m_text_match_word,
    m_text_regex, m_text_replace_with, m_text_search_down;

  wxFindReplaceData* m_frd{nullptr};

  bool m_use_regex{false};

  std::regex m_regex;
};
}; // namespace wex::factory
