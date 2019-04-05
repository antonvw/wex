////////////////////////////////////////////////////////////////////////////////
// Name:      window-data.cpp
// Purpose:   Implementation of wex::window_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wex/window-data.h>
#include <wex/managedframe.h>

wex::window_data::window_data()
{
  if (m_Parent == nullptr && wxTheApp != nullptr)
  {
    m_Parent = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
  }
}

wex::window_data& wex::window_data::button(long button)
{
  m_Button = button;
  return *this;
}

wex::window_data& wex::window_data::id(wxWindowID id) 
{
  m_Id = id;
  return *this;
}
  
wex::window_data& wex::window_data::name(const std::string& name) 
{
  m_Name = name;
  return *this;
}
  
wex::window_data& wex::window_data::parent(wxWindow* parent)
{
  m_Parent = parent;
  return *this;
}

wex::window_data& wex::window_data::pos(const wxPoint& point)
{
  m_Pos = point;
  return *this;
}
  
wex::window_data& wex::window_data::size(const wxSize& size)
{
  m_Size = size;
  return *this;
}
  
wex::window_data& wex::window_data::style(long style) 
{
  m_Style = style;
  return *this;
}

wex::window_data& wex::window_data::title(const std::string& title) 
{
  m_Title = title;
  return *this;
}
