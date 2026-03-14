////////////////////////////////////////////////////////////////////////////////
// Name:      stc-blame-margin.cpp
// Purpose:   Implementation of class wex::syntax::stc blame_margin methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/syntax/blame.h>
#include <wex/syntax/stc.h>

void wex::syntax::stc::blame_margin(const wex::blame* blame)
{
  const auto margin_blame(
    config(_("stc.margin.Text")).get(blame::margin_text_default_size()));
  const auto w(std::max(
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

  margin_text_show();
}

std::string wex::syntax::stc::margin_get_revision_renamed() const
{
  return blame::margin_renamed(this);
}
