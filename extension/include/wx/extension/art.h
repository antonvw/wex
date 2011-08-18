////////////////////////////////////////////////////////////////////////////////
// Name:      art.h
// Purpose:   Declaration of wxExStockArt class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXART_H
#define _EXART_H

#include <map>
#include <wx/artprov.h> // for wxArtID

// Only if we have a gui.
#if wxUSE_GUI

/// Offers a collection of art, mapping stock id's to art id's.
class WXDLLIMPEXP_BASE wxExStockArt
{
public:
  /// Constructor, fills the map first time it is invoked.
  wxExStockArt(wxWindowID id);

  /// If id is a stock id, returns stock bitmap from the stock art map.
  /// Check GetBitmap().IsOk for valid bitmap.
  const wxBitmap GetBitmap(
    const wxArtClient& client = wxART_OTHER, 
    const wxSize& bitmap_size = wxDefaultSize) const;
private:
  void Add(int id, const wxArtID art);
  
  static std::map<wxWindowID, wxArtID> m_ArtIDs;
  const wxWindowID m_Id;
};
#endif // wxUSE_GUI
#endif
