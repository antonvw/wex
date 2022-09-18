////////////////////////////////////////////////////////////////////////////////
// Name:      regex.h
// Purpose:   Include file for class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <regex>
#include <string>
#include <vector>

namespace wex
{
/// This class offers regular expression matching.
class regex
{
public:
  /// Type that specifies the matches.
  typedef std::vector<std::string> match_t;

  /// Callback type.
  typedef std::function<void(const match_t&)> function_t;

  /// Type that specifies a vector with regular expressions.
  typedef std::vector<std::string> regex_v_t;

  /// Type that specifies pair of regular expression and callback.
  typedef std::pair<std::string, function_t> regex_c_t;

  /// Type that specifies a vector with regular expressions and callbacks.
  typedef std::vector<regex_c_t> regex_v_c_t;

  /// This class contains the regex data.
  class data
  {
  public:
    /// Default constructor.
    data(
      const regex_c_t&      regex = {std::string(), nullptr},
      std::regex::flag_type flags = std::regex::ECMAScript);

    /// Returns function.
    const auto& function() const { return m_function; };

    /// Returns regex.
    const auto& regex() const { return m_regex; };

    /// Returns text.
    const auto& text() const { return m_text; };

  private:
    std::string m_text;
    std::regex  m_regex;
    function_t  m_function;
  };

  /// Constructor, provide regular expression string and regex flags.
  regex(
    const std::string&    regex,
    std::regex::flag_type flags = std::regex::ECMAScript);

  /// Constructor, provide regular expression string, callback and
  /// regex flags.
  regex(
    const std::string&    regex,
    function_t            f,
    std::regex::flag_type flags = std::regex::ECMAScript);

  /// Constructor, provide vector with regular expressions and
  /// regex flags.
  explicit regex(
    const regex_v_t&      regex,
    std::regex::flag_type flags = std::regex::ECMAScript);

  /// Constructor, provide vector with regular expressions, callbacks
  /// and regex flags.
  explicit regex(
    const regex_v_c_t&    regex,
    std::regex::flag_type flags = std::regex::ECMAScript);

  /// Returns reference to the requested submatch element.
  const auto& operator[](size_t pos) const { return m_matches[pos]; }

  /// Returns the last (sub)match.
  const auto& back() const { return m_matches.back(); }

  /// Returns true if matches is empty.
  bool empty() const { return m_matches.empty(); }

  /// Regular expression match.
  /// Returns:
  /// - -1: if text does not match or there is an error
  /// -  0: if text matches, but no submatches present.
  /// - >0: it fills submatches, and returns number of submatches.
  int match(const std::string& text);

  /// Returns the data element that matched.
  const auto& match_data() const { return m_data; }

  /// Returns the data regex index that matched, or -1 if no match was found.
  auto match_no() const { return m_match_no; }

  /// After match or search, replace text with replacement.
  /// Returns true if a regex is available, and regex_replace was invoked.
  bool replace(
    std::string&       text,
    const std::string& replacement,
    std::regex_constants::match_flag_type =
      std::regex_constants::format_sed) const;

  /// Regular expression search.
  /// Returns:
  /// - -1: if text does not match or there is an error
  /// -  0: if text matches, but no submatches present.
  /// - >0: it fills submatches, and returns number of submatches.
  int search(const std::string& text);

  /// Returns number of submatches.
  auto size() const { return m_matches.size(); }

private:
  enum class find_t;

  /// vector of regex data
  typedef std::vector<data> regex_t;

  int find(const std::string& text, find_t);

  const regex_t m_datas;

  data    m_data;
  match_t m_matches;
  int     m_match_no{-1};
};
} // namespace wex
