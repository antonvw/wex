////////////////////////////////////////////////////////////////////////////////
// Name:      stc/util.cpp
// Purpose:   Implementation of method wex::describe_basefields
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <charconv>
#include <numeric>
#include <sstream>

#include <wex/core/core.h>

#include "util.h"

namespace wex
{
class base_t
{
public:
  base_t(int base)
    : m_base(base) {};

  void from_chars(const std::string& word)
  {
    if (!m_is_ok)
    {
      return;
    }

    if (
      const auto res(
        std::from_chars(word.data(), word.data() + word.size(), m_val, m_base));
      res.ec != std::errc() || res.ptr != word.data() + word.size())
    {
      m_is_ok = false;
    }
  }

  void invalid() { m_is_ok = false; }

  void to_stream(std::stringstream& str, bool hex = false) const
  {
    if (m_is_ok)
    {
      if (!hex)
      {
        str << name() << "as dec: " << m_val << "\n";
      }
      else
      {
        str << name() << "as hex: " << std::hex << m_val << "\n";
      }
    }
  }

private:
  std::string name() const
  {
    switch (m_base)
    {
      case 8:
        return "oct ";
      case 10:
        return "dec ";
      case 16:
        return "hex ";
      default:
        assert(0);
    }
  }

  const int m_base{10};
  long      m_val{0};
  bool      m_is_ok{true};
};

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
    base_t base8(8), base10(10), base16(16);

    if (word.starts_with("0x") || word.starts_with("0X"))
    {
      word = word.substr(2);
      base8.invalid();
      base10.invalid();
    }
    else if (word.starts_with("0"))
    {
      word = word.substr(1);
      base10.invalid();
      base16.invalid();
    }

    base8.from_chars(word);
    base10.from_chars(word);
    base16.from_chars(word);

    base8.to_stream(stream);
    base16.to_stream(stream);
    base10.to_stream(stream, true);
  }

  if (!stream.str().empty())
  {
    clipboard_add(stream.str());
  }

  return boost::algorithm::trim_copy(stream.str());
}
} // namespace wex
