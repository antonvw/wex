////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of util methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/ex/ex-stream.h>
#include <wex/stc/stc.h>

#include "util.h"

namespace wex
{
std::string get_range(const std::string& text, int begin, int start)
{
  return boost::algorithm::replace_all_copy(
    text,
    "%LINES",
    std::to_string(begin + 1) + "," + std::to_string(start + 1));
}
} // namespace wex

void wex::expand_macro(wex::process_data& data, stc* stc)
{
  if (data.exe().find("%LINES") == std::string::npos)
  {
    return;
  }

  if (!stc->is_visual())
  {
    data.exe(get_range(
      data.exe(),
      std::max(
        1,
        stc->get_current_line() -
          std::min(
            (int)stc->GetLineCount(),
            (int)stc->get_file().ex_stream()->get_context_lines())),
      stc->get_current_line()));
  }
  else if (const std::string sel(stc->GetSelectedText()); !sel.empty())
  {
    data.exe(get_range(
      data.exe(),
      stc->LineFromPosition(stc->GetSelectionStart()),
      stc->LineFromPosition(stc->GetSelectionEnd())));
  }
  else
  {
    data.exe(
      get_range(data.exe(), stc->get_current_line(), stc->get_current_line()));
  }
}
