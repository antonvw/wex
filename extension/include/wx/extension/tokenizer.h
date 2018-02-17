////////////////////////////////////////////////////////////////////////////////
// Name:      tokenizer.h
// Purpose:   Declaration of wxExTokenizer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/dlimpexp.h>
#include <functional>
#include <string>
#define WHITESPACE_DELIMITERS " \t\r\n"

/// Offers a class that allows you to tokenize a string into
/// substrings or into some container.
class WXDLLIMPEXP_BASE wxExTokenizer
{
public:
  /// Constructor.
  wxExTokenizer(
    /// string to tokenize
    const std::string& text,
    /// delimiter characters, if no delimiter is given whitespace is used
    const std::string& delimiters = WHITESPACE_DELIMITERS,
    /// specify whether to skip empty tokens
    bool skip_empty_tokens = true);

  /// Returns total number of tokens in the string.
  size_t CountTokens() const;
  
  /// Get the delimiter which terminated the token last retrieved.
  auto GetLastDelimiter() const {return m_LastDelimiter;};
  
  /// Returns the next token, will return empty string if !HasMoreTokens().
  const std::string GetNextToken();

  /// Returns not yet tokenized part of string.
  const std::string GetString() const;

  /// Returns the current token.
  const std::string GetToken() const;

  /// Returns true if the string still contains delimiters, and so can be tokenized.
  /// A sequence of delimiters is skipped: an empty token is not returned.
  bool HasMoreTokens() const;

  /// Tokenizes the complete string into a templatized class 
  /// (e.g. vector<std::string>).
  /// Always restarts, so you can use HasMoreTokens before.
  /// Returns the filled in container.
  template <typename T> T Tokenize() {
    T tokens;
    m_TokenEndPos = 0;
    while (HasMoreTokens()) 
    {
      tokens.emplace_back(GetNextToken());
    }
    return tokens;};

  /// Tokenizes the complete string into a vector of integers.
  /// Always restarts, so you can use HasMoreTokens before.
  /// Returns the filled in vector.
  auto Tokenize() {
    std::vector <int> tokens;
    m_TokenEndPos = 0;
    while (HasMoreTokens()) 
    {
      tokens.emplace_back(std::stoi(GetNextToken()));
    }
    return tokens;};
private:
  const std::string m_Delimiters;
  const std::string m_Text;
  const bool m_SkipEmptyTokens;

  char m_LastDelimiter {0};
  size_t m_StartPos {0}, m_TokenEndPos {0}, m_TokenStartPos {0};
};
