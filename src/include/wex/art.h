////////////////////////////////////////////////////////////////////////////////
// Name:      art.h
// Purpose:   Declaration of wex::stockart class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <wx/artprov.h> // for wxArtID

namespace wex
{
  /// Offers a collection of art, mapping stock id's to art id's.
  class stockart
  {
  public:
    /// Constructor, fills the map first time it is invoked.
    stockart(wxWindowID id);

    /// If id is a stock id, returns stock bitmap from the stock art map.
    /// Check GetBitmap().is_ok for valid bitmap.
    const wxBitmap get_bitmap(
      const wxArtClient& client = wxART_OTHER, 
      const wxSize& bitmap_size = wxDefaultSize) const;
  private:
    static void add(int id, const wxArtID& art);
    
    static std::map<wxWindowID, wxArtID> m_ArtIDs;
    const wxWindowID m_id;
  };
};
