/******************************************************************************\
* File:          art.h
* Purpose:       Declaration of wxExStockArt class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXART_H
#define _EXART_H

#include <map>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h> // for wxArtID
#include <wx/stockitem.h> // for wxGetStockLabel and MNEMONIC

// Only if we have a gui.
#if wxUSE_GUI

/// Offers a collection of art, mapping stock id's to art id's.
class wxExStockArt
{
public:
  /// Constructor, fills the map first time it is invoked.
  wxExStockArt(int id);

  /// If id is a stock id, returns stock bitmap from the stock art map.
  /// Check GetBitmap().IsOk for valid bitmap.
  const wxBitmap GetBitmap(const wxSize& bitmap_size = wxSize(16, 15)) const;

  /// If id is a stock id, returns stock label.
  const wxString GetLabel(
    long flags = wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR) const;
private:
  static std::map<wxWindowID, wxArtID> m_StockArt;
  const int m_Id;
};
#endif // wxUSE_GUI
#endif
