////////////////////////////////////////////////////////////////////////////////
// Name:      stc/bind.cpp
// Purpose:   Implementation of class wex::stc method bind_all
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <charconv>
#include <numeric>
#include <sstream>

#include "util.h"

namespace wex
{
std::string describe_basefields(const std::string& number)
{
  if (number.empty())
  {
    return number;
  }

  std::stringstream stream;
  auto              word(number);

  if (const int c = word[0]; c < 32 || c > 125)
  {
    stream << "bin: " << c;
  }
  else
  {
    long base10_val, base16_val;
    bool base10_ok = false, base16_ok = false;

    if (
      std::from_chars(word.data(), word.data() + word.size(), base10_val).ec ==
      std::errc())
    {
      base10_ok = true;
    }

    if (word.starts_with("0x") || word.starts_with("0X"))
    {
      word      = word.substr(2);
      base10_ok = false;
    }

    if (
      std::from_chars(word.data(), word.data() + word.size(), base16_val, 16)
        .ec == std::errc())
    {
      base16_ok = true;
    }

    if (base10_ok || base16_ok)
    {
      if (base10_ok && !base16_ok)
      {
        stream << "hex: " << std::hex << base10_val;
      }
      else if (!base10_ok && base16_ok)
      {
        stream << "dec: " << base16_val;
      }
      else if (base10_ok && base16_ok)
      {
        stream << "dec: " << base16_val << " hex: " << std::hex << base10_val;
      }
    }
  }

  return stream.str();
}
} // namespace wex
