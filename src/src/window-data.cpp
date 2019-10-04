////////////////////////////////////////////////////////////////////////////////
// Name:      window-data.cpp
// Purpose:   Implementation of wex::window_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wex/window-data.h>
#include <wex/managedframe.h>

wex::window_data::window_data()
{
  if (m_parent == nullptr && wxTheApp != nullptr)
  {
    m_parent = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
  }
}

wex::window_data& wex::window_data::button(long button)
{
  m_button = button;
  return *this;
}

wex::window_data& wex::window_data::id(wxWindowID id) 
{
  m_id = id;
  return *this;
}
  
wex::window_data& wex::window_data::name(const std::string& name) 
{
  m_name = name;
  return *this;
}
  
wex::window_data& wex::window_data::parent(wxWindow* parent)
{
  m_parent = parent;
  return *this;
}

wex::window_data& wex::window_data::pos(const wxPoint& point)
{
  m_pos = point;
  return *this;
}
  
wex::window_data& wex::window_data::size(const wxSize& size)
{
  m_size = size;
  return *this;
}
  
wex::window_data& wex::window_data::style(long style) 
{
  m_Style = style;
  return *this;
}

wex::window_data& wex::window_data::title(const std::string& title) 
{
  m_title = title;
  return *this;
}
