////////////////////////////////////////////////////////////////////////////////
// Name:      vi/util.cpp
// Purpose:   Implementation of wex common utility methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core.h>
#include <wex/ex.h>
#include <wex/macros.h>
#include <wex/stc.h>
#include <wex/tokenizer.h>
#include <wx/defs.h>

bool wex::marker_and_register_expansion(const ex* ex, std::string& text)
{
  if (ex == nullptr)
  {
    return false;
  }

  for (tokenizer tkz(text, "'" + std::string(1, WXK_CONTROL_R), false);
       tkz.has_more_tokens();)
  {
    tkz.get_next_token();

    if (const auto rest(tkz.get_string()); !rest.empty())
    {
      // Replace marker.
      if (const char name(rest[0]); tkz.last_delimiter() == '\'')
      {
        if (const auto line = ex->marker_line(name); line >= 0)
        {
          replace_all(
            text,
            tkz.last_delimiter() + std::string(1, name),
            std::to_string(line + 1));
        }
        else
        {
          return false;
        }
      }
      // Replace register.
      else
      {
        if (const std::string reg(ex->get_macros().get_register(name));
            !reg.empty())
        {
          replace_all(
            text,
            tkz.last_delimiter() + std::string(1, name),
            name == '%' ? ex->get_stc()->get_filename().fullname() : reg);
        }
      }
    }
  }

  return true;
}
