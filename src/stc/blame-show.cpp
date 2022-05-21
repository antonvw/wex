////////////////////////////////////////////////////////////////////////////////
// Name:      blame-show.cpp
// Purpose:   Implementation of class wex::stc::blame_show
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/tokenizer.hpp>
#include <wex/core/log.h>
#include <wex/core/path.h>
#include <wex/factory/blame.h>
#include <wex/factory/lexers.h>
#include <wex/stc/stc.h>
#include <wex/stc/vcs-entry.h>

bool wex::stc::blame_show(vcs_entry* vcs)
{
  if (!vcs->get_blame().use() || vcs->std_out().empty())
  {
    log::debug("no blame (or no output)");
    return false;
  }

  log::trace("blame show") << vcs->name();

  const bool  is_empty(GetTextLength() == 0);
  std::string prev("!@#$%");
  SetWrapMode(wxSTC_WRAP_NONE);
  wex::blame* blame = &vcs->get_blame();
  bool        first = true;

  blame->line_no(-1);
  int ex_line_no = 0;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         vcs->std_out(),
         boost::char_separator<char>("\r\n")))
  {
    blame->parse(path(), it);

    if (first)
    {
      blame_margin(blame);
      first = false;
    }

    if (blame->info() != prev)
    {
      prev = blame->info();
    }
    else
    {
      blame->skip_info(true);
    }

    if (!is_visual())
    {
      blame->line_no(ex_line_no++);
    }

    if (is_empty)
    {
      add_text(blame->line_text() + "\n");
    }

    lexers::get()->apply_margin_text_style(this, blame);
  }

  return true;
}
