////////////////////////////////////////////////////////////////////////////////
// Name:      art.h
// Purpose:   Declaration of wex::art class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/artprov.h> // for wxArtID

#include <unordered_map>

namespace wex
{
/// Offers a collection of art, mapping window id's to art id's.
/// Default the stock art and material design art is used.
/// You can also choose to use your own art only.
class art
{
public:
  /// The kind of art that will be used by the wex lib.
  enum class art_t
  {
    BOTH,     ///< use stock and material art
    MATERIAL, ///< use material art
    STOCK,    ///< use stock art
    USER,     ///< use none, add you own art
  };

  // Static interface.

  /// Sets the default bitmap client (for material art).
  /// Returns false if client if not valid for material art.
  static bool default_client(const wxArtClient& c);

  /// Sets the default bitmap colour (for material art).
  /// Returns false if colour is not valid.
  static bool default_colour(const wxColour& c);

  /// Inserts art ids.
  static void insert(const std::unordered_map<wxWindowID, wxArtID>& ids);

  /// Specify the art to use.
  /// If USER is used, the current art ids are cleared,
  /// and you should insert your new ones to get
  /// bitmaps.
  static void type(art_t t);

  // Others.

  /// Constructor, provide the window id.
  explicit art(wxWindowID id);

  /// If the window id is a stock id, returns stock bitmap bundle from the stock
  /// art map. Check IsOk for valid bitmap.
  const wxBitmapBundle get_bitmap(
    const wxArtClient& client      = wxART_OTHER,
    const wxSize&      bitmap_size = wxDefaultSize,
    const wxColour&    colour      = wxNullColour) const;

private:
  const wxWindowID m_id;

  static std::unordered_map<wxWindowID, wxArtID> m_art_ids;

  static wxArtClient  m_client;
  static std::string  m_colour;
  static inline art_t m_type{art_t::BOTH};
};
}; // namespace wex
