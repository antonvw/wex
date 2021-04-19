////////////////////////////////////////////////////////////////////////////////
// Name:      link.cpp
// Purpose:   Implementation of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/item-vector.h>
#include <wex/link.h>
#include <wex/stc.h>

wex::link::link()
  : factory::link()
{
}

std::string wex::link::get_link_pairs(const std::string& text) const
{
  for (const auto& p : item_vector(stc::config_items())
                         .find<std::list<std::string>>(_("stc.link.Pairs")))
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
