////////////////////////////////////////////////////////////////////////////////
// Name:      regex.h
// Purpose:   Include file for class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

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
    /// Constructor, provide single regular expression, and optional callback.
    regex(
      const std::string&    regex,
      std::function<void()> f     = nullptr,
      std::regex::flag_type flags = std::regex::ECMAScript);

    /// Constructor, provide vector with regular expressions.
    regex(
      const std::vector<std::string>& regex,
      std::regex::flag_type           flags = std::regex::ECMAScript);

    /// Constructor, provide vector with regular expressions and callbacks.
    regex(
      const std::vector<std::pair<std::string, std::function<void()>>>& regex,
      std::regex::flag_type flags = std::regex::ECMAScript);

    /// Regular expression match.
    /// Returns:
    /// - -1: if text does not match or there is an error
    /// -  0: if text matches, but no submatches present.
    /// - >0: it fills submatches.
    int match(const std::string& text);

    /// Returns the (sub)matches.
    const auto& matches() const { return m_matches; };

    /// Returns reference to the requested submatch element.
    const std::string& operator[](size_t pos) const { return m_matches[pos]; };

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
    auto size() const { return m_matches.size(); };

    /// Returns the tuple element that matched.
    const auto& which() const { return m_which; };

    /// Returns the regex tuple no that matched, or -1 if no match was found.
    auto which_no() const { return m_which_no; };

  private:
    enum class find_t;

    /// a regex element: tuple regex, callback, regex string
    typedef std::tuple<std::regex, std::function<void()>, std::string>
      regex_e_t;

    /// vector of regex elements
    typedef std::vector<regex_e_t> regex_t;

    int find(const std::string& text, find_t);

    const regex_t m_regex;
    int           m_which_no{-1};

    std::vector<std::string> m_matches;
    regex_e_t                m_which;
  };
} // namespace wex
