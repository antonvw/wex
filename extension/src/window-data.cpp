////////////////////////////////////////////////////////////////////////////////
// Name:      stc-wnidow.cpp
// Purpose:   Implementation of wex::window_data
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wx/extension/window-data.h>
#include <wx/extension/managedframe.h>

wex::window_data::window_data()
{
  if (m_Parent == nullptr && wxTheApp != nullptr)
  {
    m_Parent = dynamic_cast<managed_frame*>(wxTheApp->GetTopWindow());
  }
}

wex::window_data& wex::window_data::Button(long button)
{
  m_Button = button;
  return *this;
}

wex::window_data& wex::window_data::Id(wxWindowID id) 
{
  m_Id = id;
  return *this;
}
  
wex::window_data& wex::window_data::Name(const std::string& name) 
{
  m_Name = name;
  return *this;
}
  
wex::window_data& wex::window_data::Parent(wxWindow* parent)
{
  m_Parent = parent;
  return *this;
}

wex::window_data& wex::window_data::Pos(const wxPoint& point)
{
  m_Pos = point;
  return *this;
}
  
wex::window_data& wex::window_data::Size(const wxSize& size)
{
  m_Size = size;
  return *this;
}
  
wex::window_data& wex::window_data::Style(long style) 
{
  m_Style = style;
  return *this;
}

wex::window_data& wex::window_data::Title(const std::string& title) 
{
  m_Title = title;
  return *this;
}
