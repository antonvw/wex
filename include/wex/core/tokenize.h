////////////////////////////////////////////////////////////////////////////////
// Name:      tokenize.h
// Purpose:   Declaration of wex::tokenize method
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/tokenizer.hpp>

#include <string>

namespace wex
{
/// Tokenizes the complete string into a templatized class
/// (e.g. vector<std::string>).
/// Returns the filled in container.
template <typename T>
T tokenize(const std::string& text, const char* sep = " \t\r\n")
{
  T tokens;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         text,
         boost::char_separator<char>(sep)))
  {
    tokens.emplace_back(it);
  }

  return tokens;
};
} // namespace wex
