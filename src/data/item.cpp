////////////////////////////////////////////////////////////////////////////////
// Name:      data/item.cpp
// Purpose:   Implementation of wex::data::item
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/data/item.h>

wex::data::item::item(const data::control& data)
  : m_data(data)
{
}

wex::data::item& wex::data::item::apply(const user_apply_t& rhs)
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

wex::data::item& wex::data::item::initial(const std::any& rhs)
{
  m_initial = rhs;
  return *this;
}

wex::data::item& wex::data::item::is_persistent(bool rhs)
{
  m_is_persistent = rhs;
  return *this;
}

wex::data::item& wex::data::item::is_readonly(bool rhs)
{
  m_is_readonly = rhs;
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

wex::data::item& wex::data::item::validate(const user_validate_t& rhs)
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
wex::data::item::user_window_create(const user_window_create_t& rhs)
{
  m_user_window_create_t = rhs;
  return *this;
}

wex::data::item&
wex::data::item::user_window_to_config(const user_window_to_config_t& rhs)
{
  m_user_window_to_config_t = rhs;
  return *this;
}

wex::data::item& wex::data::item::window(const data::window& rhs)
{
  m_data.window(rhs);
  return *this;
}
