/******************************************************************************\
* File:          art.cpp
* Purpose:       Implementation of wxExStockArt class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/art.h>

#if wxUSE_GUI

std::map<wxWindowID, wxArtID> wxExStockArt::m_StockArt;

wxExStockArt::wxExStockArt(wxWindowID id)
  : m_Id(id)
{
  if (m_StockArt.empty())
  {
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_BACKWARD, wxART_GO_BACK));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_CLOSE, wxART_CLOSE));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_COPY, wxART_COPY));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_CUT, wxART_CUT));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_DELETE, wxART_DELETE));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_DOWN, wxART_GO_DOWN));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_EXIT, wxART_QUIT));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_FIND, wxART_FIND));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_FORWARD, wxART_GO_FORWARD));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_HELP, wxART_HELP));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_HOME, wxART_GO_HOME));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_NEW, wxART_NEW));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_OPEN, wxART_FILE_OPEN));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_PASTE, wxART_PASTE));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_PRINT, wxART_PRINT));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_REDO, wxART_REDO));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_REPLACE, wxART_FIND_AND_REPLACE));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_SAVE, wxART_FILE_SAVE));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_SAVEAS, wxART_FILE_SAVE_AS));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_UNDO, wxART_UNDO));
    m_StockArt.insert(std::make_pair((wxWindowID)wxID_UP, wxART_GO_UP));
  }
}

const wxBitmap wxExStockArt::GetBitmap(
  const wxArtClient& client,
  const wxSize& bitmap_size) const
{
  wxBitmap bitmap;

  if (wxIsStockID(m_Id))
  {
    // Check if there is art for this id.
    const auto art_it = m_StockArt.find(m_Id);

    if (art_it != m_StockArt.end())
    {
      bitmap = wxArtProvider::GetBitmap(
        art_it->second, 
        client, 
        bitmap_size);
    }
  }

  return bitmap;
}
#endif // wxUSE_GUI
