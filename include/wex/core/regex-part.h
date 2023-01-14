////////////////////////////////////////////////////////////////////////////////
// Name:      regex-part.h
// Purpose:   Include file for class wex::regex_part
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <regex>
#include <string>

namespace wex
{
/// This class offers partial regular expression matching.
/// For example a regex: \*+ Settings \*+
/// and incomplete text '* Set' would not match a normal std::regex,
/// this class matches all *, * , * Set, * Settings *,
/// where the first 3 would return a MATCH_ALL, and the last a MATCH_COMPLETE.
class regex_part
{
public:
  /// The match types.
  enum match_t
  {
    MATCH_NONE,     ///< the current text does not match at all, even partly
    MATCH_ERROR,    ///< the regex part gives a regex error during parsing
    MATCH_COMPLETE, ///< the current text matches the regex
    MATCH_PART,     ///< the current text matches part of regex part
    MATCH_ALL,      ///< the current text matches all regex part
  };

  /// Constructor.
  regex_part(
    /// the regular expression string
    const std::string& regex,
    /// the regex flags
    std::regex::flag_type flags = std::regex::ECMAScript);

  /// Returns the error (in case MATCH_ERROR).
  const auto& error() const { return m_error; };

  /// Regular expression match using a single char at a time,
  /// possibly continuing a previous match.
  /// Returns the match_t:
  /// - MATCH_ALL: The resulting text from this and all previous chars do match.
  /// - MATCH_COMPLETE: As MATCH_ALL, and it matches the complete regex.
  /// - MATCH_PART: Part of the supplied chars match, the last one doesn't
  /// - MATCH_NONE: There is no match, or a regex error
  match_t match(char c);

  /// Returns the match type.
  auto match_type() const { return m_match_type; };

  /// Returns the regex part the text matches with until now.
  const auto& part() const { return m_regex_part; };

  /// Returns the regex.
  const auto& regex() const { return m_regex; };

  /// Resets partial matching to start again.
  void reset();

  /// Returns the current text, matching depending of the current match type.
  const auto& text() const { return m_text; };

private:
  const std::string regex_next();

  const std::string           m_regex;
  const std::regex::flag_type m_flags;

  bool    m_is_matching{false};
  match_t m_match_type{MATCH_NONE};

  std::string::const_iterator m_it;
  std::string                 m_error, m_regex_part, m_text;
};

// implementation

inline wex::regex_part::regex_part(
  const std::string&    regex,
  std::regex::flag_type flags)
  : m_regex(regex)
  , m_it(m_regex.begin())
  , m_flags(flags)
{
}

inline wex::regex_part::match_t wex::regex_part::match(char c)
{
  if (m_match_type == MATCH_PART || m_regex.empty() || !isascii(c))
  {
    return m_match_type;
  }

  if (m_regex_part.empty())
  {
    regex_next();
  }

  m_text.push_back(c);

  try
  {
    if (std::match_results<std::string::const_iterator> m;
        !std::regex_match(m_text, m, std::regex(m_regex_part, m_flags)) &&
        !std::regex_match(m_text, m, std::regex(regex_next(), m_flags)))
    {
      m_match_type = m_is_matching ? MATCH_PART : MATCH_NONE;
      return m_match_type;
    }
  }
  catch (std::regex_error& e)
  {
    m_error = m_regex_part + " error: " + e.what() + "\n";

    m_match_type = MATCH_ERROR;
    return m_match_type;
  }

  m_is_matching = true;

  m_match_type = (m_regex_part == m_regex) ? MATCH_COMPLETE : MATCH_ALL;

  return m_match_type;
}

inline const std::string wex::regex_part::regex_next()
{
  bool bracket = false, parentheses = false, escape = false;

  while (m_it != m_regex.end())
  {
    m_regex_part.push_back(*m_it);

    switch (*m_it++)
    {
      case '(':
        parentheses = true;
        break;

      case ')':
        return m_regex_part;
        break;

      case '[':
        bracket = true;
        break;

      case ']':
        return m_regex_part;
        break;

      case '?':
        break;

      case '\\':
        break;

      default:
        if (!bracket && !parentheses)
        {
          if (
            m_it != m_regex.end() && *m_it != '*' && *m_it != '+' &&
            *m_it != '?')
          {
            return m_regex_part;
          }
        }
    }
  }

  return m_regex_part;
}

inline void wex::regex_part::reset()
{
  m_error.clear();
  m_it          = m_regex.begin();
  m_is_matching = false;
  m_match_type  = MATCH_NONE;
  m_regex_part.clear();
  m_text.clear();
};

}; // namespace wex
