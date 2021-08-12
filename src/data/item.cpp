////////////////////////////////////////////////////////////////////////////////
// Name:      data/item.cpp
// Purpose:   Implementation of wex::data::item
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/item.h>

wex::data::item::item(const data::control& data)
  : m_data(data)
{
}

wex::data::item::item(const item& r, const std::any& initial)
  : m_initial(initial)
{
  *this = r;
}

wex::data::item& wex::data::item::operator=(const data::item& r)
{
  if (this != &r)
  {
    m_apply                   = r.m_apply;
    m_data                    = r.m_data;
    m_image_list              = r.m_image_list;
    m_inc                     = r.m_inc;
    m_is_regex                = r.m_is_regex;
    m_label_type              = r.m_label_type;
    m_min                     = r.m_min;
    m_major_dimension         = r.m_major_dimension;
    m_max                     = r.m_max;
    m_user_window_create_t    = r.m_user_window_create_t;
    m_user_window_to_config_t = r.m_user_window_to_config_t;
    m_validate                = r.m_validate;
    m_validate_re             = r.m_validate_re;

    if (r.m_initial.has_value())
    {
      m_initial = r.m_initial;
    }
  }

  return *this;
}

wex::data::item& wex::data::item::apply(const user_apply_t rhs)
{
  m_apply = rhs;
  return *this;
}

wex::data::item& wex::data::item::columns(int rhs)
{
  m_major_dimension = rhs;
  return *this;
}

wex::data::item& wex::data::item::control(const data::control& rhs)
{
  m_data = rhs;
  return *this;
}

wex::data::item& wex::data::item::inc(const std::any& rhs)
{
  m_inc = rhs;
  return *this;
}

wex::data::item& wex::data::item::is_regex(bool rhs)
{
  m_is_regex = rhs;
  return *this;
}

wex::data::item& wex::data::item::label_type(label_t rhs)
{
  m_label_type = rhs;
  return *this;
}

wex::data::item& wex::data::item::min(const std::any& rhs)
{
  m_min = rhs;
  return *this;
}

wex::data::item& wex::data::item::max(const std::any& rhs)
{
  m_max = rhs;
  return *this;
}

wex::data::item& wex::data::item::image_list(wxImageList* il)
{
  m_image_list = il;
  return *this;
}

wex::data::item& wex::data::item::validate(const user_validate_t rhs)
{
  m_validate = rhs;
  return *this;
}

wex::data::item& wex::data::item::validate_re(const std::string& rhs)
{
  m_validate_re = rhs;
  return *this;
}

wex::data::item&
wex::data::item::user_window_create(const user_window_create_t rhs)
{
  m_user_window_create_t = rhs;
  return *this;
}

wex::data::item&
wex::data::item::user_window_to_config(const user_window_to_config_t rhs)
{
  m_user_window_to_config_t = rhs;
  return *this;
}

wex::data::item& wex::data::item::window(const data::window& rhs)
{
  m_data.window(rhs);
  return *this;
}
