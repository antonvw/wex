////////////////////////////////////////////////////////////////////////////////
// Name:      tokenize.h
// Purpose:   Declaration of wex::tokenize method
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <boost/tokenizer.hpp>

#include <algorithm>
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

  const auto& in(boost::tokenizer<boost::char_separator<char>>(
    text,
    boost::char_separator<char>(sep)));

  std::copy(in.begin(), in.end(), back_inserter(tokens));

  return tokens;
};
} // namespace wex
