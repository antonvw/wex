////////////////////////////////////////////////////////////////////////////////
// Name:      notebook-data.h
// Purpose:   Implementation of wex::data::notebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/notebook-data.h>
#include <wx/window.h>

wex::data::notebook& wex::data::notebook::bitmap(const wxBitmap& rhs)
{
  m_bitmap = rhs;

  return *this;
}

wex::data::notebook& wex::data::notebook::caption(const std::string& rhs)
{
  m_caption = rhs;

  return *this;
}

wex::data::notebook& wex::data::notebook::index(size_t rhs)
{
  m_page_index = rhs;

  return *this;
}

wex::data::notebook& wex::data::notebook::key(const std::string& rhs)
{
  m_key = rhs;

  return *this;
}

wex::data::notebook& wex::data::notebook::page(wxWindow* rhs)
{
  m_page = rhs;

  return *this;
}

wex::data::notebook& wex::data::notebook::select()
{
  m_select = true;

  return *this;
}
