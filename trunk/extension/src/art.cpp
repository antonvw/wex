/******************************************************************************\
* File:          art.cpp
* Purpose:       Implementation of wxExStockArt class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: base.cpp 1677 2009-09-19 18:02:00Z antonvw $
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/art.h>

using namespace std;

#if wxUSE_GUI

map<wxWindowID, wxArtID> wxExStockArt::m_StockArt;

wxExStockArt::wxExStockArt()
{
  if (m_StockArt.empty())
  {
    m_StockArt.insert(make_pair((wxWindowID)wxID_BACKWARD, wxART_GO_BACK));
    m_StockArt.insert(make_pair((wxWindowID)wxID_COPY, wxART_COPY));
    m_StockArt.insert(make_pair((wxWindowID)wxID_CUT, wxART_CUT));
    m_StockArt.insert(make_pair((wxWindowID)wxID_DELETE, wxART_DELETE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_DOWN, wxART_GO_DOWN));
    m_StockArt.insert(make_pair((wxWindowID)wxID_EXIT, wxART_QUIT));
    m_StockArt.insert(make_pair((wxWindowID)wxID_FIND, wxART_FIND));
    m_StockArt.insert(make_pair((wxWindowID)wxID_FORWARD, wxART_GO_FORWARD));
    m_StockArt.insert(make_pair((wxWindowID)wxID_HELP, wxART_HELP));
    m_StockArt.insert(make_pair((wxWindowID)wxID_HOME, wxART_GO_HOME));
    m_StockArt.insert(make_pair((wxWindowID)wxID_NEW, wxART_NEW));
    m_StockArt.insert(make_pair((wxWindowID)wxID_OPEN, wxART_FILE_OPEN));
    m_StockArt.insert(make_pair((wxWindowID)wxID_PASTE, wxART_PASTE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_PRINT, wxART_PRINT));
    m_StockArt.insert(make_pair((wxWindowID)wxID_REDO, wxART_REDO));
    m_StockArt.insert(make_pair((wxWindowID)wxID_REPLACE, wxART_FIND_AND_REPLACE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_SAVE, wxART_FILE_SAVE));
    m_StockArt.insert(make_pair((wxWindowID)wxID_SAVEAS, wxART_FILE_SAVE_AS));
    m_StockArt.insert(make_pair((wxWindowID)wxID_UNDO, wxART_UNDO));
    m_StockArt.insert(make_pair((wxWindowID)wxID_UP, wxART_GO_UP));
    m_StockArt.insert(make_pair((wxWindowID)wxID_VIEW_DETAILS, wxART_REPORT_VIEW));
    m_StockArt.insert(make_pair((wxWindowID)wxID_VIEW_LIST, wxART_LIST_VIEW));
  }
}

void wxExStockArt::CheckStock(
  int id,
  wxString& stock_label,
  wxBitmap& bitmap,
  long flags,
  const wxSize& bitmap_size)
{
  if (wxIsStockID(id))
  {
    if (!stock_label.empty())
    {
      wxLogWarning(wxString::Format(
        "You specified a label: %s, though there is a stock label for it",
        stock_label.c_str()));
    }

    stock_label = wxGetStockLabel(id, flags);

    // Check if there is art for this id.
    map<wxWindowID, wxArtID>::const_iterator art_it = m_StockArt.find(id);

    if (art_it != m_StockArt.end())
    {
      if (bitmap.IsOk())
      {
        wxLogWarning(wxString::Format(
          "You specified art: %s, though there is stock art for it",
          stock_label.c_str()));
      }

      bitmap = wxArtProvider::GetBitmap(art_it->second, wxART_MENU, bitmap_size);
    }
  }
}
#endif // wxUSE_GUI
