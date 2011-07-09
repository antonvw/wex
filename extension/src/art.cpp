////////////////////////////////////////////////////////////////////////////////
// Name:      art.cpp
// Purpose:   Implementation of wxExStockArt class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/art.h>

#if wxUSE_GUI

std::map<wxWindowID, wxArtID> wxExStockArt::m_ArtIDs;

wxExStockArt::wxExStockArt(wxWindowID id)
  : m_Id(id)
{
  if (m_ArtIDs.empty())
  {
    Add(wxID_BACKWARD, wxART_GO_BACK);
    Add(wxID_CLOSE, wxART_CLOSE);
    Add(wxID_COPY, wxART_COPY);
    Add(wxID_CUT, wxART_CUT);
    Add(wxID_DELETE, wxART_DELETE);
    Add(wxID_DOWN, wxART_GO_DOWN);
    Add(wxID_EXIT, wxART_QUIT);
    Add(wxID_FIND, wxART_FIND);
    Add(wxID_FIRST, wxART_GOTO_FIRST);
    Add(wxID_FORWARD, wxART_GO_FORWARD);
    Add(wxID_HELP, wxART_HELP);
    Add(wxID_HOME, wxART_GO_HOME);
    Add(wxID_INFO, wxART_INFORMATION);
    Add(wxID_LAST, wxART_GOTO_LAST);
    Add(wxID_NEW, wxART_NEW);
    Add(wxID_OPEN, wxART_FILE_OPEN);
    Add(wxID_PASTE, wxART_PASTE);
    Add(wxID_PRINT, wxART_PRINT);
    Add(wxID_REDO, wxART_REDO);
    Add(wxID_REPLACE, wxART_FIND_AND_REPLACE);
    Add(wxID_SAVE, wxART_FILE_SAVE);
    Add(wxID_SAVEAS, wxART_FILE_SAVE_AS);
    Add(wxID_UNDO, wxART_UNDO);
    Add(wxID_UP, wxART_GO_UP);
    Add(wxID_VIEW_DETAILS, wxART_REPORT_VIEW);
    Add(wxID_VIEW_LIST, wxART_LIST_VIEW);
  }
}

void wxExStockArt::Add(int id, const wxArtID art)
{    
  m_ArtIDs.insert(std::make_pair((wxWindowID)id, art));
}

const wxBitmap wxExStockArt::GetBitmap(
  const wxArtClient& client,
  const wxSize& bitmap_size) const
{
  if (wxIsStockID(m_Id))
  {
    // Check if there is art for this id.
    const auto art_it = m_ArtIDs.find(m_Id);

    if (art_it != m_ArtIDs.end())
    {
      return wxArtProvider::GetBitmap(
        art_it->second, 
        client, 
        bitmap_size);
    }
  }

  return wxBitmap();
}
#endif // wxUSE_GUI
