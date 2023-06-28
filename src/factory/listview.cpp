////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of wex core listview methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/factory/listview.h>

wex::column::column()
{
  SetColumn(-1);
}

wex::column::column(const std::string& name, type_t type, int width)
  : m_type(type)
{
  wxListColumnFormat align;

  switch (m_type)
  {
    case column::FLOAT:
      align = wxLIST_FORMAT_RIGHT;
      if (width == 0)
        width = 80;
      break;

    case column::INT:
      align = wxLIST_FORMAT_RIGHT;
      if (width == 0)
        width = 60;
      break;

    case column::STRING:
      align = wxLIST_FORMAT_LEFT;
      if (width == 0)
        width = 100;
      break;

    case column::DATE:
      align = wxLIST_FORMAT_LEFT;
      if (width == 0)
        width = 150;
      break;

    default:
      assert(0);
  }

  SetColumn(-1); // default value, is set when inserting the col
  SetText(name);
  SetAlign(align);
  SetWidth(width);
}

void wex::column::set_is_sorted_ascending(sort_t type)
{
  switch (type)
  {
    case SORT_ASCENDING:
      m_is_sorted_ascending = true;
      break;

    case SORT_DESCENDING:
      m_is_sorted_ascending = false;
      break;

    case SORT_KEEP:
      break;

    case SORT_TOGGLE:
      m_is_sorted_ascending = !m_is_sorted_ascending;
      break;

    default:
      assert(0);
      break;
  }
}

wex::factory::listview::listview(
  const data::window&  window,
  const data::control& control)
  : wxListView(
      window.parent(),
      window.id(),
      window.pos(),
      window.size(),
      window.style() == data::NUMBER_NOT_SET ? wxLC_REPORT : window.style(),
      control.validator() != nullptr ? *control.validator() :
                                       wxDefaultValidator,
      window.name())
{
}
