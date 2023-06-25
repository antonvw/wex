////////////////////////////////////////////////////////////////////////////////
// Name:      regex-part.h
// Purpose:   Include file for class wex::regex_part
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/regex.hpp>
#include <string>

namespace wex
{
/// This class offers partial regular expression matching, as supported
/// by the boost regex classes.
/// For example a regex: \*+ Settings \*+
/// and incomplete text '* Set' would not match a normal std::regex,
/// this class matches all *, * , * Set, * Settings *,
/// where the first 3 would return a MATCH_PART, and the last a MATCH_FULL.
class regex_part
{
public:
  /// The match types.
  enum match_t
  {
    MATCH_NONE,  ///< the current text does not match at all, even partly
    MATCH_ERROR, ///< the regex part gives a regex error during parsing
    MATCH_PART,  ///< the current text matches part of regex part
    MATCH_FULL,  ///< the current text matches all regex part
  };

  /// Constructor.
  regex_part(
    /// the regular expression string
    const std::string& regex,
    /// the regex flags
    boost::regex::flag_type flags = boost::regex::ECMAScript);

  /// Returns the error (in case MATCH_ERROR).
  const std::string& error() const { return m_error; };

  /// Regular expression match using a single char at a time,
  /// possibly continuing a previous match.
  /// Returns the match_t:
  /// - MATCH_PART: The resulting text from this and all previous chars do match
  /// - MATCH_FULL: The text matches the complete regex
  /// - MATCH_NONE: There is no match
  /// - MATCH_ERROR: There is a regex error, available using error()
  /// Non ascii chars are ignored.
  match_t match(char c);

  /// Returns the match type.
  match_t match_type() const { return m_match_type; };

  /// Returns the regex.
  const std::string& regex() const { return m_regex; };

  /// Resets matching to start again.
  void reset();

  /// Returns the current text (all chars offered to match).
  const std::string& text() const { return m_text; };

private:
  boost::regex::flag_type m_flags;

  match_t m_match_type{MATCH_NONE};

  std::string m_error, m_regex, m_text;
};

// implementation

inline wex::regex_part::regex_part(
  const std::string&      regex,
  boost::regex::flag_type flags)
  : m_regex(regex)
  , m_flags(flags)
{
}

inline wex::regex_part::match_t wex::regex_part::match(char c)
{
  if (m_regex.empty() || !isascii(c))
  {
    return m_match_type;
  }

  m_text.push_back(c);

  try
  {
    if (boost::match_results<std::string::const_iterator> m;
        !boost::regex_match(
          m_text,
          m,
          boost::regex(m_regex, m_flags),
          boost::match_default | boost::match_partial))
    {
      m_match_type = MATCH_NONE;
    }
    else
    {
      m_match_type = m[0].matched ? MATCH_FULL : MATCH_PART;
    }
  }
  catch (boost::regex_error& e)
  {
    m_error      = m_regex + " error: " + e.what() + "\n";
    m_match_type = MATCH_ERROR;
  }

  return m_match_type;
}

inline void wex::regex_part::reset()
{
  m_error.clear();
  m_match_type = MATCH_NONE;
  m_text.clear();
};

}; // namespace wex
