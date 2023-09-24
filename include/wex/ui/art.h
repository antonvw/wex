////////////////////////////////////////////////////////////////////////////////
// Name:      art.h
// Purpose:   Declaration of wex::stockart class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID

#include <unordered_map>

namespace wex
{
/// Offers a collection of art, mapping stock id's to art id's.
class stockart
{
public:
  /// Constructor, fills the map first time it is invoked.
  explicit stockart(wxWindowID id);

  /// If id is a stock id, returns stock bitmap bundle from the stock art map.
  /// Check getBitmap().is_ok for valid bitmap.
  const wxBitmapBundle get_bitmap(
    const wxArtClient& client      = wxART_OTHER,
    const wxSize&      bitmap_size = wxDefaultSize) const;

private:
  static void add(int id, const wxArtID& art);

  static inline std::unordered_map<wxWindowID, wxArtID> m_art_ids;
  const wxWindowID                                      m_id;
};
}; // namespace wex
