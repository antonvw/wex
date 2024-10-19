////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of wex core listview methods
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/core.h>
#include <wex/factory/listview.h>

#define SETUP_COL(NAME, ALIGN, SIZE)                                           \
  case column::NAME:                                                           \
    align = ALIGN;                                                             \
                                                                               \
    if (width == 0)                                                            \
    {                                                                          \
      width = config("col." #NAME).get(SIZE);                                  \
    }                                                                          \
    break

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
    SETUP_COL(DATE, wxLIST_FORMAT_LEFT, 150);
    SETUP_COL(FLOAT, wxLIST_FORMAT_RIGHT, 80);
    SETUP_COL(INT, wxLIST_FORMAT_RIGHT, 60);
    SETUP_COL(STRING, wxLIST_FORMAT_LEFT, 100);

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
