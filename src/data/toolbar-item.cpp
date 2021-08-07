////////////////////////////////////////////////////////////////////////////////
// Name:      toolbar.cpp
// Purpose:   Implementation of wex::data::toolbar_item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/toolbar-item.h>

wex::data::toolbar_item& wex::data::toolbar_item::bitmap(const wxBitmap& rhs)
{
  m_bitmap = rhs;

  return *this;
}

wex::data::toolbar_item& wex::data::toolbar_item::kind(wxItemKind rhs)
{
  m_kind = rhs;

  return *this;
}

wex::data::toolbar_item& wex::data::toolbar_item::label(const std::string& rhs)
{
  m_label = rhs;

  return *this;
}

wex::data::toolbar_item& wex::data::toolbar_item::help(const std::string& rhs)
{
  m_short_help = rhs;

  return *this;
}
