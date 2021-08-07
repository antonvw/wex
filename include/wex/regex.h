////////////////////////////////////////////////////////////////////////////////
// Name:      regex.h
// Purpose:   Include file for class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <regex>
#include <string>
#include <tuple>
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

    /// Type that specifies a vector with regular expressions and callbacks.
    typedef std::vector<std::pair<std::string, function_t>> regex_v_c_t;

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
    regex(
      const regex_v_t&      regex,
      std::regex::flag_type flags = std::regex::ECMAScript);

    /// Constructor, provide vector with regular expressions, callbacks
    /// and regex flags.
    regex(
      const regex_v_c_t&    regex,
      std::regex::flag_type flags = std::regex::ECMAScript);

    /// Regular expression match.
    /// Returns:
    /// - -1: if text does not match or there is an error
    /// -  0: if text matches, but no submatches present.
    /// - >0: it fills submatches.
    int match(const std::string& text);

    /// Returns the (sub)matches.
    const auto& matches() const { return m_matches; }

    /// Returns reference to the requested submatch element.
    const std::string& operator[](size_t pos) const { return m_matches[pos]; }

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
    /// - >0: it fills submatches.
    int search(const std::string& text);

    /// Returns number of submatches.
    auto size() const { return m_matches.size(); }

    /// Returns the tuple element that matched.
    const auto& which() const { return m_which; }

    /// Returns the regex tuple no that matched, or -1 if no match was found.
    auto which_no() const { return m_which_no; }

  private:
    enum class find_t;

    /// a regex element: tuple regex, callback, regex string
    typedef std::tuple<std::regex, function_t, std::string> regex_e_t;

    /// vector of regex elements
    typedef std::vector<regex_e_t> regex_t;

    int find(const std::string& text, find_t);

    const regex_t m_regex;
    int           m_which_no{-1};

    match_t   m_matches;
    regex_e_t m_which;
  };
} // namespace wex
