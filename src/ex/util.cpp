////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of wex ex utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef __WXGTK__
#include <format>
#endif
#include <boost/regex.hpp>

#include <wex/core/log.h>
#include <wex/ex/ex.h>
#include <wex/ex/macros.h>
#include <wex/ex/util.h>
#include <wex/syntax/stc.h>

#include "util.h"

void wex::append_line_no(std::string& text, int line)
{
  text += std::format("{:6} ", line + 1);
}

const std::string wex::esc()
{
  return std::string("\x1b");
}

const std::string wex::find_first_of(
  const std::string& text,
  const std::string& chars,
  size_t             pos)
{
  const auto& match = text.find_first_of(chars, pos);
  return match == std::string::npos ? std::string() : text.substr(match + 1);
}

std::string
wex::get_lines(factory::stc* stc, int start, int end, const std::string& flags)
{
  if (start == end)
  {
    return std::string();
  }

  std::string text;

  for (auto i = start; i < end && i < stc->get_line_count(); i++)
  {
    if (flags.contains("#"))
    {
      append_line_no(text, i);
    }

    if (flags.contains("l"))
    {
      text += stc->GetTextRange(
                stc->PositionFromLine(i),
                stc->GetLineEndPosition(i)) +
              "$\n";
    }
    else
    {
      text += stc->GetLine(i);
    }
  }

  if (!text.ends_with("\n"))
  {
    text += "\n";
  }

  return text;
}

bool wex::is_register_valid(const std::string& text)
{
  return text.size() == 2 && (text[0] == '@' || text[0] == WXK_CONTROL_R) &&
         boost::regex_match(
           text,
           boost::regex("^" + std::string(1, text[0]) + "[0-9=\"a-z%._\\*]$"));
}

const std::string wex::k_s(wxKeyCode key)
{
  return std::string(1, key);
}

bool wex::marker_and_register_expansion(const ex* ex, std::string& text)
{
  if (ex == nullptr)
  {
    return false;
  }

  // Replace any register in text with contents and any marker with line no.
  // E.g. xxx 11  22 'x control_ry
  //      xxx 11  22 5 7
  // where 'x contains 5 and register y contains 7
  std::string output;
  bool        changed = false;

  try
  {
    for (auto it = text.begin(); it != text.end(); ++it)
    {
      switch (*it)
      {
        // Replace marker.
        case '\'':
        case '`':
          if (auto next = std::next(it); next == text.end())
          {
            /* NOLINTNEXTLINE */
            output += *it;
          }
          else if (const auto line = ex->marker_line(*(next)); line >= 0)
          {
            output += std::to_string(line + 1);
            ++it;
            changed = true;
          }
          else
          {
            /* NOLINTNEXTLINE */
            output += *it;
          }
          break;

        // Replace register.
        case WXK_CONTROL_R:
          if (*std::next(it) == '%')
          {
            output += ex->get_stc()->path().filename();
          }
          else
          {
            const auto next(std::next(it));
            if (next == text.end())
            {
              log("missing register") << text;
              return false;
            }

            const auto& reg(ex->get_macros().get_register(*(next)));
            output += reg;
          }

          ++it;
          changed = true;
          break;

        default:
          output += *it;
      }
    }
  }
  catch (std::exception& e)
  {
    log(e) << text;
    return false;
  }

  if (changed)
  {
    text = output;
  }

  return true;
}

std::string wex::to_reverse(const std::string& text)
{
  std::string s(text);
  std::ranges::transform(
    s,
    std::begin(s),
    [](const auto& c)
    {
      return std::islower(c) ? std::toupper(c) : std::tolower(c);
    });
  return s;
}
