////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.cpp
// Purpose:   Implementation of class wex::factory::hexmode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/hex.hpp>
#include <wex/factory/hexmode.h>
#include <wex/factory/stc.h>

wex::factory::hexmode::hexmode(factory::stc* stc, size_t bytes_per_line)
  : m_stc(stc)
  , m_bytes_per_line(bytes_per_line)
  , m_each_hex_field(3)
{
}

void wex::factory::hexmode::activate()
{
  m_is_active = true;
}

void wex::factory::hexmode::deactivate()
{
  m_is_active = false;
}

const std::string
wex::factory::hexmode::line(const std::string& text, size_t offset) const
{
  std::stringstream field_hex, field_ascii;

  auto count = text.size() - offset;
  count      = (m_bytes_per_line < count ? m_bytes_per_line : count);

  for (size_t byte = 0; byte < count; byte++)
  {
    field_hex << boost::algorithm::hex(std::string(1, text[offset + byte]))
              << " ";
    field_ascii << printable(text[offset + byte]);
  }

  const auto field_spaces =
    std::string((m_bytes_per_line - count) * m_each_hex_field, ' ');

  return field_hex.str() + field_spaces + field_ascii.str() +
         (text.size() - offset > m_bytes_per_line ? m_stc->eol() :
                                                    std::string());
}

const std::string wex::factory::hexmode::lines(const std::string& text) const
{
  std::string output;

  if (is_active())
  {
    output.reserve(
      // number of lines
      m_stc->eol().size() * (text.size() / bytes_per_line()) +
      // hex field
      each_hex_field() * text.size() +
      // ascii field
      text.size());

    for (size_t offset = 0; offset < text.size(); offset += bytes_per_line())
    {
      output += line(text, offset);
    }
  }

  return output;
}

char wex::factory::hexmode::printable(unsigned int c) const
{
  // We do not want control chars (\n etc.) to be printed,
  // as that disturbs the hex view field.
  if ((isascii(c) && !iscntrl(c)) || !is_active())
  {
    return c;
  }

  // If we already defined our own symbol, use that one,
  // otherwise print an ordinary ascii char.
  const int symbol = m_stc->GetControlCharSymbol();
  return symbol == 0 ? '.' : symbol;
}
