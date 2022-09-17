////////////////////////////////////////////////////////////////////////////////
// Name:      stc.cpp
// Purpose:   Implementation of class wex::stc blame margin methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/syntax/blame.h>
#include <wex/stc/stc.h>
#include <wex/ui/item-vector.h>

void wex::stc::blame_margin(const wex::blame* blame)
{
  const item_vector iv(m_config_items);
  const auto        margin_blame(iv.find<int>(_("stc.margin.Text")));
  const int         w(std::max(
    config(_("stc.Default font"))
        .get(wxFont(
          12,
          wxFONTFAMILY_DEFAULT,
          wxFONTSTYLE_NORMAL,
          wxFONTWEIGHT_NORMAL))
        .GetPixelSize()
        .GetWidth() +
      1,
    5));

  SetMarginWidth(
    m_margin_text_number,
    margin_blame == -1 ? blame->info().size() * w : margin_blame);
}

std::string wex::stc::margin_get_revision_renamed() const
{
  return blame::margin_renamed(this);
}
