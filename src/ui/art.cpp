////////////////////////////////////////////////////////////////////////////////
// Name:      art.cpp
// Purpose:   Implementation of wex::stockart class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/defs.h>
#include <wex/ui/art.h>
#include <wx/stockitem.h>
#include <wxMaterialDesignArtProvider.hpp>

std::unordered_map<wxWindowID, wxArtID> wex::stockart::m_art_ids{
  // stock items, with wx art, or material art
  {wxID_ADD, wxART_ADD},
  {wxID_BACKWARD, wxART_GO_BACK},
  {wxID_CLEAR, wxART_CLEANING_SERVICES},
  {wxID_CLOSE, wxART_CLOSE},
  {wxID_COPY, wxART_COPY},
  {wxID_CUT, wxART_CUT},
  {wxID_DELETE, wxART_DELETE},
  {wxID_DOWN, wxART_GO_DOWN},
  {wxID_EDIT, wxART_EDIT},
  {wxID_EXECUTE, wxART_EXECUTABLE_FILE},
  {wxID_EXIT, wxART_QUIT},
  {wxID_FIND, wxART_FIND},
  {wxID_FIRST, wxART_GOTO_FIRST},
  {wxID_FORWARD, wxART_GO_FORWARD},
  {wxID_HELP, wxART_HELP},
  {wxID_HOME, wxART_GO_HOME},
  {wxID_INFO, wxART_INFORMATION},
  {wxID_LAST, wxART_GOTO_LAST},
  {wxID_NEW, wxART_NEW},
  {wxID_OPEN, wxART_FILE_OPEN},
  {wxID_PASTE, wxART_PASTE},
  {wxID_PREVIEW, wxART_PREVIEW},
  {wxID_PRINT, wxART_PRINT},
  {wxID_REDO, wxART_REDO},
  {wxID_REFRESH, wxART_REFRESH},
  {wxID_REPLACE, wxART_FIND_AND_REPLACE},
  {wxID_SAVE, wxART_FILE_SAVE},
  {wxID_SAVEAS, wxART_FILE_SAVE_AS},
  {wxID_SELECTALL, wxART_SELECT_ALL},
  {wxID_STOP, wxART_STOP},
  {wxID_UNDO, wxART_UNDO},
  {wxID_UP, wxART_GO_UP},
  {wxID_VIEW_DETAILS, wxART_REPORT_VIEW},
  {wxID_VIEW_LIST, wxART_LIST_VIEW},

  // wex items, with material art
  {ID_CLEAR_FILES, wxART_CLEANING_SERVICES},
  {ID_CLEAR_FINDS, wxART_CLEANING_SERVICES},
  {ID_CLEAR_PROJECTS, wxART_CLEANING_SERVICES},
  {ID_EDIT_SELECT_INVERT, wxART_INVERT_COLORS},
  {ID_EDIT_SELECT_NONE, wxART_DESELECT},
  {ID_TOOL_ADD, wxART_ADD},
  {ID_TOOL_REPLACE, wxART_FIND_REPLACE},
  {ID_TOOL_REPORT_FIND, wxART_FIND_IN_PAGE}};

wxArtClient wex::stockart::m_client = wxART_CLIENT_MATERIAL_ROUND;
std::string wex::stockart::m_colour = "light blue";

wex::stockart::stockart(wxWindowID id)
  : m_id(id)
{
}

bool wex::stockart::default_client(const wxArtClient& c)
{
  if (!wxMaterialDesignArtProvider::HasClient(c))
  {
    return false;
  }

  m_client = c;

  return true;
}

bool wex::stockart::default_colour(const wxColour& c)
{
  if (!c.IsOk())
  {
    return false;
  }

  m_colour = c.GetAsString();

  return true;
}

void wex::stockart::insert(const std::unordered_map<wxWindowID, wxArtID>& ids)
{
  for (const auto& id : ids)
  {
    m_art_ids.insert(id);
  }
}

const wxBitmapBundle wex::stockart::get_bitmap(
  const wxArtClient& client,
  const wxSize&      bitmap_size,
  const wxColour&    colour) const
{
  if (const auto& art_it = m_art_ids.find(m_id); art_it != m_art_ids.end())
  {
    if (m_type == art_t::BOTH || m_type == art_t::STOCK)
    {
      if (wxIsStockID(m_id))
      {
        if (const auto& bb(wxArtProvider::GetBitmapBundle(
              art_it->second,
              client,
              bitmap_size));
            bb.IsOk())
        {
          return bb;
        }
      }
    }

    if (m_type == art_t::BOTH || m_type == art_t::MATERIAL)
    {
      if (const auto& bitmap(wxMaterialDesignArtProvider::GetBitmap(
            art_it->second,
            m_client,
            bitmap_size,
            colour.IsOk() ? colour : wxColour(m_colour)));
          bitmap.IsOk())
      {
        return bitmap;
      }
    }

    if (m_type == art_t::USER)
    {
      return wxArtProvider::GetBitmapBundle(
        art_it->second,
        client,
        bitmap_size);
    }
  }

  return wxBitmapBundle();
}

void wex::stockart::type(art_t t)
{
  if (t == art_t::USER)
  {
    m_art_ids.clear();
  }

  m_type = t;
}
