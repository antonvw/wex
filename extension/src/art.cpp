////////////////////////////////////////////////////////////////////////////////
// Name:      art.cpp
// Purpose:   Implementation of wex::stockart class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/art.h>

std::map<wxWindowID, wxArtID> wex::stockart::m_ArtIDs;

wex::stockart::stockart(wxWindowID id)
  : m_Id(id)
{
  if (m_ArtIDs.empty())
  {
    add(wxID_BACKWARD, wxART_GO_BACK);
    add(wxID_CLOSE, wxART_CLOSE);
    add(wxID_COPY, wxART_COPY);
    add(wxID_CUT, wxART_CUT);
    add(wxID_DELETE, wxART_DELETE);
    add(wxID_DOWN, wxART_GO_DOWN);
#if wxCHECK_VERSION(3,1,0)
    add(wxID_EDIT, wxART_EDIT);
#endif
    add(wxID_EXECUTE, wxART_EXECUTABLE_FILE);
    add(wxID_EXIT, wxART_QUIT);
    add(wxID_FIND, wxART_FIND);
    add(wxID_FIRST, wxART_GOTO_FIRST);
    add(wxID_FORWARD, wxART_GO_FORWARD);
    add(wxID_HELP, wxART_HELP);
    add(wxID_HOME, wxART_GO_HOME);
    add(wxID_INFO, wxART_INFORMATION);
    add(wxID_LAST, wxART_GOTO_LAST);
    add(wxID_NEW, wxART_NEW);
    add(wxID_OPEN, wxART_FILE_OPEN);
    add(wxID_PASTE, wxART_PASTE);
    add(wxID_PRINT, wxART_PRINT);
    add(wxID_REDO, wxART_REDO);
    add(wxID_REPLACE, wxART_FIND_AND_REPLACE);
    add(wxID_SAVE, wxART_FILE_SAVE);
    add(wxID_SAVEAS, wxART_FILE_SAVE_AS);
    add(wxID_UNDO, wxART_UNDO);
    add(wxID_UP, wxART_GO_UP);
    add(wxID_VIEW_DETAILS, wxART_REPORT_VIEW);
    add(wxID_VIEW_LIST, wxART_LIST_VIEW);
  }
}

void wex::stockart::add(int id, const wxArtID art)
{    
  m_ArtIDs.insert({(wxWindowID)id, art});
}

const wxBitmap wex::stockart::get_bitmap(
  const wxArtClient& client,
  const wxSize& bitmap_size) const
{
  if (wxIsStockID(m_Id))
  {
    // Check if there is art for this id.
    const auto& art_it = m_ArtIDs.find(m_Id);

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
