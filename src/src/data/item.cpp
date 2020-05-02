////////////////////////////////////////////////////////////////////////////////
// Name:      data/item.cpp
// Purpose:   Implementation of wex::item_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/item-data.h>

wex::item_data::item_data(const control_data& data)
{
  m_data = data;
}

wex::item_data::item_data(const item_data& r, const std::any& initial)
  : m_initial(initial)
{
  *this = r;
}

wex::item_data& wex::item_data::operator=(const item_data& r)
{
  if (this != &r)
  {
    m_apply                   = r.m_apply;
    m_data                    = r.m_data;
    m_image_list              = r.m_image_list;
    m_inc                     = r.m_inc;
    m_label_type              = r.m_label_type;
    m_min                     = r.m_min;
    m_major_dimension         = r.m_major_dimension;
    m_max                     = r.m_max;
    m_user_window_create_t    = r.m_user_window_create_t;
    m_user_window_to_config_t = r.m_user_window_to_config_t;

    if (r.m_initial.has_value())
    {
      m_initial = r.m_initial;
    }
  }

  return *this;
}

wex::item_data& wex::item_data::apply(const user_apply_t rhs)
{
  m_apply = rhs;
  return *this;
}

wex::item_data& wex::item_data::columns(int rhs)
{
  m_major_dimension = rhs;
  return *this;
}

wex::item_data& wex::item_data::control(const control_data& rhs)
{
  m_data = rhs;
  return *this;
}

wex::item_data& wex::item_data::inc(const std::any& rhs)
{
  m_inc = rhs;
  return *this;
}

wex::item_data& wex::item_data::label_type(label_t rhs)
{
  m_label_type = rhs;
  return *this;
}

wex::item_data& wex::item_data::min(const std::any& rhs)
{
  m_min = rhs;
  return *this;
}

wex::item_data& wex::item_data::max(const std::any& rhs)
{
  m_max = rhs;
  return *this;
}

wex::item_data& wex::item_data::image_list(wxImageList* il)
{
  m_image_list = il;
  return *this;
}

wex::item_data&
wex::item_data::user_window_create(const user_window_create_t rhs)
{
  m_user_window_create_t = rhs;
  return *this;
}

wex::item_data&
wex::item_data::user_window_to_config(const user_window_to_config_t rhs)
{
  m_user_window_to_config_t = rhs;
  return *this;
}

wex::item_data& wex::item_data::window(const window_data& rhs)
{
  m_data.window(rhs);
  return *this;
}
