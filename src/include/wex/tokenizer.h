////////////////////////////////////////////////////////////////////////////////
// Name:      tokenizer.h
// Purpose:   Declaration of wex::tokenizer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <string>
#include <vector>

namespace wex
{
  const std::string WHITESPACE_DELIMITERS = " \t\r\n";

  /// Offers a class that allows you to tokenize a string into
  /// substrings or into some container.
  class tokenizer
  {
  public:
    /// Constructor.
    tokenizer(
      /// string to tokenize
      const std::string& text,
      /// delimiter characters, if no delimiter is given whitespace is used
      const std::string& delimiters = WHITESPACE_DELIMITERS,
      /// specify whether to skip empty tokens
      bool skip_empty_tokens = true);

    /// Returns total number of tokens in the string.
    size_t count_tokens() const;

    /// Returns the next token, will return empty string if !has_more_tokens().
    const std::string get_next_token();

    /// Returns not yet tokenized part of string.
    const std::string get_string() const;

    /// Returns the current token.
    const std::string get_token() const;

    /// Returns true if the string still contains delimiters, and so can be
    /// tokenized. A sequence of delimiters is skipped: an empty token is not
    /// returned.
    bool has_more_tokens() const;

    /// Get the delimiter which terminated the token last retrieved.
    auto last_delimiter() const { return m_last_delimiter; };

    /// tokenizes the complete string into a templatized class
    /// (e.g. vector<std::string>).
    /// Always restarts, so you can use has_more_tokens before.
    /// Returns the filled in container.
    template <typename T> T tokenize()
    {
      T tokens;
      m_token_end_pos = 0;
      while (has_more_tokens())
      {
        tokens.emplace_back(get_next_token());
      }
      return tokens;
    };

    /// tokenizes the complete string into a vector of integers (size_t).
    /// Always restarts, so you can use has_more_tokens before.
    /// Returns the filled in vector.
    auto tokenize()
    {
      std::vector<size_t> tokens;
      m_token_end_pos = 0;
      while (has_more_tokens())
      {
        tokens.emplace_back(std::stoi(get_next_token()));
      }
      return tokens;
    };

  private:
    const std::string m_delimiters, m_text;

    const bool m_skip_empty_tokens;

    char m_last_delimiter{0};

    size_t m_start_pos{0}, m_token_end_pos{0}, m_token_start_pos{0};
  };
}; // namespace wex
