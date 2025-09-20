////////////////////////////////////////////////////////////////////////////////
// Name:      data/window.cpp
// Purpose:   Implementation of wex::data::window
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/factory/window.h>
#include <wx/app.h>
#include <wx/window.h>

wex::data::window::window()
{
  if (m_parent == nullptr && wxTheApp != nullptr)
  {
    m_parent = wxTheApp->GetTopWindow();
  }
}

wex::data::window&
wex::data::window::allow_move_path_extension(const std::string& rhs)
{
  m_allow_move_path_extension = rhs;
  return *this;
}

wex::data::window& wex::data::window::button(long button)
{
  m_button = button;
  return *this;
}

wex::data::window& wex::data::window::id(wxWindowID id)
{
  m_id = id;
  return *this;
}

wex::data::window& wex::data::window::name(const std::string& name)
{
  m_name = name;
  return *this;
}

wex::data::window& wex::data::window::parent(wxWindow* parent)
{
  m_parent = parent;
  return *this;
}

wex::data::window& wex::data::window::pos(const wxPoint& point)
{
  m_pos = point;
  return *this;
}

wex::data::window& wex::data::window::size(const wxSize& size)
{
  m_size = size;
  return *this;
}

wex::data::window& wex::data::window::style(long style)
{
  m_style = style;
  return *this;
}

wex::data::window& wex::data::window::title(const std::string& title)
{
  m_title = title;
  return *this;
}

wex::data::window& wex::data::window::wildcard(const std::string& rhs)
{
  m_wildcard = rhs;
  return *this;
}
