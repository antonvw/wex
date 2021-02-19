////////////////////////////////////////////////////////////////////////////////
// Name:      regex.h
// Purpose:   Include file for class wex::regex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <regex>
#include <string>
#include <vector>

namespace wex
{
  /// This class offers regular expression matching.
  class regex
  {
  public:
    /// Constructor, provide single regular expression.
    explicit regex(
      const std::string&    regex,
      std::regex::flag_type flags = std::regex::ECMAScript);

    /// Constructor, provide vector with regular expressions.
    explicit regex(
      const std::vector<std::string>& regex,
      std::regex::flag_type           flags = std::regex::ECMAScript);

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

    /// Returns number of submatches.
    auto size() const { return m_matches.size(); };

    /// Returns the regex pair that matched.
    const auto& which() const { return m_which; };

    /// Returns the regex pair no that matched, or -1 if no match was found.
    auto which_no() const { return m_which_no; };

  private:
    /// vector of pair regex, regex string
    typedef std::vector<std::pair<std::regex, std::string>> regex_t;

    const regex_t m_regex;
    int           m_which_no;

    std::vector<std::string>           m_matches;
    std::pair<std::regex, std::string> m_which;
  };
} // namespace wex
