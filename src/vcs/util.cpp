////////////////////////////////////////////////////////////////////////////////
// Name:      util.cpp
// Purpose:   Implementation of util methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ex/ex-stream.h>
#include <wex/factory/frame.h>
#include <wex/stc/stc.h>
#include <wex/syntax/lexers.h>
#include <wex/vcs/unified-diff.h>
#include <wex/vcs/vcs.h>

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
  if (!data.exe().contains("%LINES"))
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

bool wex::vcs_diff(const std::string& command)
{
  return command == "diff" &&
         config(_("vcs.Use unified diff view")).get(true) &&
         lexers::get()->is_loaded();
}

bool wex::vcs_execute(
  factory::frame*          frame,
  int                      id,
  const std::vector<path>& files,
  const data::window&      data)
{
  if (files.empty())
  {
    return false;
  }

  if (vcs vcs(files, id); vcs.entry().get_command().is_open())
  {
    if (vcs.show_dialog(data) == wxID_OK)
    {
      std::ranges::for_each(
        files,
        [frame, id](const auto& it)
        {
          if (wex::vcs vcs({it}, id); vcs.execute())
          {
            if (!vcs.entry().std_out().empty())
            {
              if (vcs_diff(vcs.entry().get_command().get_command()))
              {
                unified_diff(it, &vcs.entry(), frame).parse();
              }
              else
              {
                frame->open_file_vcs(it, vcs.entry(), data::stc());
              }
            }
            else if (!vcs.entry().std_err().empty())
            {
              log() << vcs.entry().std_err();
            }
            else
            {
              log::status("No output");
              log::debug("no output from") << vcs.entry().data().exe();
            }
          }
        });
    }
  }
  else
  {
    vcs.request();
  }

  return true;
}
