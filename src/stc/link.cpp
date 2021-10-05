////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/stc/link.h>
#include <wex/stc/stc.h>
#include <wex/stc/vcs.h>
#include <wex/ui/item-vector.h>

wex::link::link()
  : factory::link()
{
}

std::string wex::link::get_link_pairs(const std::string& text) const
{
  for (const auto& p : item_vector(stc::config_items())
                         .find<config::strings_t>(_("stc.link.Pairs")))
  {
    const auto pos1 = text.find(before(p, '\t'));
    const auto pos2 = text.rfind(after(p, '\t'));

    if (pos1 != std::string::npos && pos2 != std::string::npos && pos2 > pos1)
    {
      // Okay, get everything in between, and make sure we skip white space.
      return boost::algorithm::trim_copy(
        text.substr(pos1 + 1, pos2 - pos1 - 1));
    }
  }

  return std::string();
}

const wex::path
wex::link::get_path(const std::string& text, line_data& data, factory::stc* stc)
{
  if (vcs v; v.use() && v.toplevel().dir_exists())
  {
    add_path(v.toplevel());
  }

  return factory::link::get_path(text, data, stc);
}
