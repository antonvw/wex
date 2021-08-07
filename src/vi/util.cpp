////////////////////////////////////////////////////////////////////////////////
// Name:      vi/util.cpp
// Purpose:   Implementation of wex common utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/ex.h>
#include <wex/macros.h>
#include <wex/stc.h>
#include <wx/defs.h>

bool wex::marker_and_register_expansion(const ex* ex, std::string& text)
{
  if (ex == nullptr)
  {
    return false;
  }

  // Replace any register in text with contents and any marker with line no.
  // E.g. xxx 11  22 'x control_ry
  //      xxx 11  22 5 7
  // if 'x contains 5 and register y contains 7
  std::string output;
  bool        changed = false;

  for (auto it = text.begin(); it != text.end(); ++it)
  {
    switch (*it)
    {
      // Replace marker.
      case '\'':
        if (const auto line = ex->marker_line(*(std::next(it))); line >= 0)
        {
          output += std::to_string(line + 1);
          ++it;
          changed = true;
        }
        else
        {
          output += *it;
        }
        break;

      // Replace register.
      case WXK_CONTROL_R:
        if (*std::next(it) == '%')
        {
          output += ex->get_stc()->get_filename().fullname();
        }
        else 
        {
          const std::string reg(
                   ex->get_macros().get_register(*(std::next(it))));
          output += reg;
        }
      
        ++it;
        changed = true;
        break;

      default:
        output += *it;
    }
  }

  if (changed)
  {
    text = output;
  }

  return true;
}
